#include <QApplication>
#include <QImageReader>
#include <QColorSpace>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QString>

#include <iostream>

#include "core/ImageLoader.h"
#include "core/ImageProcessor.h"

#include "gui/MainWindow.h"

// ---------------------- Helper ----------------------
static void showHistory() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/history.json";
    QFile file(path);
    if ( !file.open(QIODevice::ReadOnly) ) {
        std::cout << "No history available." << std::endl;
        return;
    }
    QJsonArray history = QJsonDocument::fromJson(file.readAll()).array();
    std::cout << "--- Last calls ---" << std::endl;
    for ( const QJsonValue &val : history ) {
        QJsonObject obj = val.toObject();
        std::cout << obj["date"].toString().toStdString() << " | " 
                  << "ImageEditor "
                  << obj["args"].toString().toStdString() << std::endl;
    }
}

static void saveCurrentCall( int argc, char *argv[] ) {
  // std::cout << "saveCurrentCall(): Processing..." << std::endl;
  {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    std::cout << " path=" << path.toStdString() << std::endl;
    QDir().mkpath(path); 
    QString filePath = path + "/history.json";
    QFile file(filePath);
    QJsonArray history;
    if ( file.open(QIODevice::ReadOnly) ) {
        history = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
    QStringList argList;
    for( int i = 1; i < argc; ++i ) argList << argv[i];
    QJsonObject currentCall;
    currentCall.insert("date", QDateTime::currentDateTime().toString(Qt::ISODate));
    currentCall.insert("args", argList.join(" "));
    history.prepend(currentCall);
    if ( history.size() > 100 ) history.removeLast();
    if ( file.open(QIODevice::WriteOnly) ) {
        file.write(QJsonDocument(history).toJson());
    }
  }
}

static void printError( const QString &msg ) {
    // \033[1;31m makes it BOLD and RED
    std::cerr << "\033[1;31m" << "ERROR: " << "\033[0m" 
              << msg.toStdString() << std::endl;
}

static bool validateFile( const QString &filePath, const QString &optionName, const QStringList &allowedExtensions = {} ) {
  auto errorPrefix = []() { return "\033[1;31mERROR: \033[0m"; };
  if ( !filePath.isEmpty() ) {
    QFileInfo fileInfo(filePath);
    if ( !fileInfo.exists() ) {
        std::cerr << errorPrefix() << "File '" << filePath.toStdString() << "' does not exist." << std::endl;
        return false;
    } if ( !fileInfo.isFile() ) {
        std::cerr << errorPrefix() << "'" << filePath.toStdString() << "' is a directory, not a file." << std::endl;
        return false;
    } if ( !fileInfo.isReadable() ) {
        std::cerr << errorPrefix() << "File '" << filePath.toStdString() << "' is not readable (check permissions)." << std::endl;
        return false;
    }
    if ( !allowedExtensions.isEmpty() ) {
     QString suffix = fileInfo.suffix().toLower();
     if ( !allowedExtensions.contains(suffix) ) {
      std::cerr << errorPrefix() << "Invalid format for --" << optionName.toStdString() 
                      << ". Allowed: " << allowedExtensions.join(", ").toStdString() << "." << std::endl;
      return false;
     }
    }
  }
  return true;
}

// ---------------------- Command Line Options ----------------------
static QJsonObject parser( const QCoreApplication *app, int argc ) {
  QJsonObject obj;
  QCommandLineParser parser;
  parser.setApplicationDescription("A simple ImageEditor with JSON-history support.");
  parser.addHelpOption();
  parser.addVersionOption();   
  QCommandLineOption fileOption(QStringList() << "f" << "file", "Path to input image file.", "file");
  parser.addOption(fileOption);
  QCommandLineOption projectFileOption(QStringList() << "project", "Path to input JSON-project file.", "json");
  parser.addOption(projectFileOption);
  QCommandLineOption outFileOption(QStringList() << "o" << "output", "Path to output image file.", "file");
  parser.addOption(outFileOption);
  QCommandLineOption batchOption("batch", "Run application in batch mode without running graphical user interface.");
  parser.addOption(batchOption);
  QCommandLineOption vulkanOption("vulkan", "If available enable hardware accelerated Vulkan rendering.");
  parser.addOption(vulkanOption);
  QCommandLineOption historyOption("history", "Print history of last calls to stdout.");
  parser.addOption(historyOption);
  parser.process(*app);
  
  // --- history ---
  if ( parser.isSet(historyOption) && argc == 2 ) {
   showHistory();
   exit(1);
  }
  // --- Check required options ---
  if ( !parser.isSet(fileOption) && !parser.isSet(projectFileOption) ) {
   qCritical() << "Error: Missing path to image file and history file. Need at least one!";
   parser.showHelp();
  }
  // --- Set variables ---
  obj["imagePath"] = parser.value(fileOption);
  if ( !validateFile(obj["imagePath"].toString(),"image file",{"png","mnc","mnc2","tif","tiff"}) ) {
    exit(1);
  }
  obj["outputPath"] = parser.value(outFileOption);
  obj["imagePath"] = parser.value(fileOption);
  if ( !validateFile(obj["imagePath"].toString(),"image file",{"png","mnc","mnc2","tif","tiff"}) ) {
   exit(1);
  }
  obj["historyPath"] = parser.value(projectFileOption);
  if ( !validateFile(obj["historyPath"].toString(),"project",{"json"}) ) {
   exit(1);
  }
  obj["vulkan"] = parser.isSet(vulkanOption);
  
  return obj;
}

// ---------------------- Main ----------------------
int main( int argc, char *argv[] )
{
    // --- prepare (required for Linux.Debian systems in batch mode) ---
    if ( getenv("DISPLAY") == nullptr ) {
      setenv("QT_QPA_PLATFORM", "offscreen", 1);
    }
    QImageReader::setAllocationLimit(0); // dangerous
    
    // --- check first for gui option ---
    bool batchProcssing = false;
    for ( int i=0 ; i<argc ; ++i ) {
     if ( QString(argv[i]) == "--batch" ) batchProcssing = true;
    }
    
    // --- check whether batch processing is requested---
    if ( batchProcssing ) {
      QCoreApplication *app = new QCoreApplication(argc,argv);
      app->setApplicationName("ImageEditor");
      app->setApplicationVersion("1.0");
      QJsonObject parsedOptions = parser(app,argc);
      QString imagePath = parsedOptions.value("imagePath").toString("");
      QString historyPath = parsedOptions.value("historyPath").toString("");
      if ( historyPath.isEmpty() ) {
       qInfo() << "Error: Invalid input. Missing required option '--project <filename>' in batch mode.";
       return 1;
      }
      QString outputPath = parsedOptions.value("outputPath").toString("");
      if ( outputPath.isEmpty() ) {
       qInfo() << "Error: Invalid input. Missing required option '--output <filename>' in batch mode.";
       return 1;
      }
      ImageLoader loader;
      QImage image;
      if ( imagePath.isEmpty() ) {
       saveCurrentCall(argc, argv);
       ImageProcessor proc;
       if ( !proc.process(historyPath) ) {
        qInfo() << "Error: Malfunction in ImageProcessor::process(" << historyPath << ").";
        return 1;
       }
       image = proc.getOutputImage();
      } else {
       if ( loader.load(imagePath,true) ) {
        saveCurrentCall(argc, argv);
        ImageProcessor proc(loader.getImage());
        if ( !proc.process(historyPath) ) {
         qInfo() << "Error: Malfunction in ImageProcessor::process(" << historyPath << ").";
         return 1;
        }
        image = proc.getOutputImage();
       } else {
        qInfo() << "Error: Malfunction in ImageLoader::load(" << imagePath << ").";
        return 0;
       }
      }
      image.setColorSpace(QColorSpace(QColorSpace::SRgb)); // no change
      if ( loader.saveAs(image,outputPath) ) {
       qInfo() << "Saved image file '" << outputPath << "'.";
       return 1;
      }
      return 1;
    }
    
    // --- gui processing ---
    QApplication *app = new QApplication(argc, argv);
    app->setApplicationName("ImageEditor");
    app->setApplicationVersion("1.0");
    QJsonObject parsedOptions = parser(app,argc);
    QString imagePath = parsedOptions.value("imagePath").toString("");
    QString historyPath = parsedOptions.value("historyPath").toString("");
    QString outputPath = parsedOptions.value("outputPath").toString("");
    bool useVulkan = parsedOptions.value("vulkan").toBool();
    
    // --- create new history entry ---
    saveCurrentCall(argc, argv);
    
    // --- call main programm ---
    MainWindow w(imagePath,historyPath,useVulkan);
    w.show();
    return app->exec();
    
}