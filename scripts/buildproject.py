"""

  Description:
  
      The buildproject.py script is an automation tool designed to generate structured project JSON files 
      for the ImageEditor application. It parses a command-based batch file (.bat) and combines it with 
      image assets to create a ready-to-use project file. The script handles complex tasks like binary 
      image encoding, MD5 checksum generation, and metadata extraction automatically.
      
  Limitations:
  
      The script accepts files of type png and minc. However, minc files have not been tested and there 
      is therefore no guarantee that they will work.
      
  Key Features:
  
      Automatic Base64 Encoding: Converts PNG images into Base64 strings, matching the Qt QByteArray to toBase64 workflow.

      Binary Header Inspection: Reads PNG headers directly to extract width and height without requiring heavy imaging libraries.

      Checksum & Timestamps: Automatically generates MD5 hashes and file timestamps for the MainImage (ID 0).

      Command Mapping: Translates Create Layer instructions into both layers entries and LassoCutCommand undo-steps.
      
  Mandatory Parameters:
  
      To build a project, you must provide the following three arguments:

          --input <path> Path to the primary background image (becomes the "MainImage" with ID 0).

          --bat <path> Path to the .bat file containing the layer creation and movement instructions.

          --output <filename> The desired name for the resulting JSON project file.
          
  Example Command:

      Bash: 'python3 buildproject.py --input <FILENAME> --bat script.bat --output project.json'
      
  .bat File Syntax Rules:
  
      The buildproject.py script identifies so far two main types of commands within the batch file. Any line not starting 
      with these keywords is ignored.
      
      1. The Create Layer Command

           This command initializes a new layer, embeds the image data, and records the initial "cut" position.

           Syntax: Create Layer[ID] image from [PATH] at [X][Y]

           Components:

               Layer[ID]: The integer ID assigned to the layer (e.g., Layer1).

               from [PATH]: The relative or absolute path to the PNG file for this layer.

               at [X][Y]: The initial coordinates using the + or - prefix (e.g., +1000+1000).

           Example: Create Layer1 image from images/head.png at +500+200
           
      2. The MoveLayer Command
      
           This command adds a movement step to the undoStack for an existing layer.

           Syntax: MoveLayer [ID] to [X][Y]

           Components:

               [ID]: The integer ID of the layer to move.

               to [X][Y]: The target coordinates (e.g., +3001+2883).

           Example: MoveLayer 1 to +1200+1500
      
  Utility Commands:

       --history [N] Displays a list of previous builds. Providing a number N limits the output to the last N entries.

       --repeat <index> Instantly re-runs a build process using the parameters from the specified history index.

  Author:
  
      Hartmut Mohlberg, INM-1, Research Center Juelich, h.mohlberg@fz-juelich.de

"""

import json
import argparse
import os
import hashlib
import time
import re
import base64
import struct
import sys
from datetime import datetime

HISTORY_FILE = ".buildproject_history.json"
SUPPORTED_EXTENSIONS = ('.png', '.minc')

def is_supported(filename):
    """Prüft, ob die Dateiendung erlaubt ist."""
    return filename.lower().endswith(SUPPORTED_EXTENSIONS)

def get_history():
    """ Lädt die Historie aus der Datei. """
    if os.path.exists(HISTORY_FILE):
        try:
            with open(HISTORY_FILE, 'r', encoding='utf-8') as f:
                return json.load(f)
        except:
            return []
    return []
    
def save_to_history(args_dict):
    history = get_history()
    history.append({
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "input": args_dict.get('input'),
        "bat": args_dict.get('bat'),
        "output": args_dict.get('output')
    })
    with open(HISTORY_FILE, 'w', encoding='utf-8') as f:
        json.dump(history[-100:], f, indent=4)

def show_history(n=None):
    history = get_history()
    if not history:
        print("No history available.")
        return
    limit = n if n is not None else len(history)
    to_show = history[-limit:]
    print(f"\n--- Build history (Last {len(to_show)}) ---")
    start_idx = len(history) - len(to_show)
    for i, e in enumerate(to_show, start_idx + 1):
        print(f"{i:2d}. [{e['timestamp']}] --input {e['input']} --bat {e['bat']} --output {e['output']}")
    print("-" * 40)

def get_image_size(fname):
    """Liest Dimensionen aus PNG. Bei MINC oder Fehlern Rückgabe 0,0."""
    if not os.path.exists(fname): return 0, 0
    if fname.lower().endswith('.png'):
        with open(fname, 'rb') as f:
            data = f.read(24)
            if data[:8] == b'\x89PNG\r\n\x1a\n' and data[12:16] == b'IHDR':
                w, h = struct.unpack('>LL', data[16:24])
                return int(w), int(h)
    # Für .minc oder unbekannte Header
    return 0, 0
    
def get_md5(fname):
    hash_md5 = hashlib.md5()
    if os.path.exists(fname):
        with open(fname, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()
    return ""
    
def get_image_base64(img_path):
    """Entspricht dem Qt-Workflow: Image -> PNG Buffer -> Base64 String."""
    if not os.path.exists(img_path):
        print(f"Warning: Layer-image {img_path} not found.")
        return ""
    with open(img_path, "rb") as img_file:
        # Da das Bild bereits ein PNG ist, lesen wir es direkt binär
        # und wandeln es in einen Base64-kodierten UTF-8 String um.
        return base64.b64encode(img_file.read()).decode('utf-8')

def parse_bat_line(line):
    # Extrahiert Koordinaten wie +1000+1000 oder -50-100
    coords = re.findall(r'[+-]\d+', line)
    x = int(coords[0]) if len(coords) > 0 else 0
    y = int(coords[1]) if len(coords) > 1 else 0
    return x, y

def build_project(input_img, bat_file, output_json):
    # Validierung des Hauptbildes
    if not is_supported(input_img):
        print(f"Error: Format of '{input_img}' is not supported. Allowed: {SUPPORTED_EXTENSIONS}")
        return False
    
    if not os.path.exists(input_img):
        print(f"Error: Main image '{input_img}' not found.")
        return False
    
    project = {"layers": [], "undoStack": []}
    last_positions = {}
    abs_path = os.path.abspath(input_img)
    
    # Main Image
    project["layers"].append({
        "filename": os.path.basename(input_img),
        "filetime": time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(os.path.getmtime(input_img))),
        "id": "0", "md5checksum": get_md5(input_img), "name": "MainImage", "pathname": os.path.dirname(abs_path)
    })

    try:
        with open(bat_file, 'r') as f:
            lines = f.readlines()
        
        layer_count = 0
        for line in [l.strip() for l in lines if l.strip()]:
            if line.startswith("Create Layer"):
                match_id = re.search(r'Layer(\d+)', line)
                match_path = re.search(r'from\s+(.*?)\s+at', line)
                if match_id and match_path:
                    layer_id = int(match_id.group(1))
                    img_path = match_path.group(1).strip()
                    
                    x, y = parse_bat_line(line)
                    w, h = get_image_size(img_path)
                    
                    # Startposition für diesen Layer merken
                    last_positions[layer_id] = (x, y)
                    
                    project["layers"].append({
                        "data": get_image_base64(img_path), "id": layer_id,
                        "name": f"Lasso layer {layer_id}", "opacity": 1
                    })
                    project["undoStack"].append({
                        "name": "Lasso layer", "newLayerId": layer_id, "originalLayerId": 0,
                        "rect": {"height": h, "width": w, "x": x, "y": y},
                        "text": "Lasso layer Cut", "type": "LassoCutCommand"
                    })
                    layer_count += 1
            
            # MOVE LAYER
            elif line.startswith("MoveLayer"):
                parts = line.split()
                if len(parts) >= 2:
                    layer_id = int(parts[1])
                    to_x, to_y = parse_bat_line(line)
                    from_x, from_y = last_positions.get(layer_id, (0, 0))
                    project["undoStack"].append({
                        "fromX": from_x, "fromY": from_y,
                        "layerId": layer_id,
                        "text": f"Move Layer {layer_id}",
                        "toX": to_x, "toY": to_y, "type": "MoveLayer"
                    })
                    last_positions[layer_id] = (to_x, to_y)

        with open(output_json, 'w') as f:
            json.dump(project, f, indent=4)
        
        # Zusammenfassung
        file_size_mb = os.path.getsize(output_json) / (1024 * 1024)
        print(f"Success: {output_json} created.")
        print(f" -> Number of layers created: {layer_count}")
        print(f" -> File size: {file_size_mb:.2f} MB")
        return True
    except Exception as e:
        print(f"Fehler: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Build ImageEditor project JSON from image and bat.")
    parser.add_argument("--input", help="Path to main image (*.png,*.minc)")
    parser.add_argument("--bat", help="Path to bat file")
    parser.add_argument("--output", help="Output JSON path")
    parser.add_argument("--history", nargs='?', const=-1, type=int, help="Show history")
    parser.add_argument("--repeat", type=int, help="Repeat from history index")

    args = parser.parse_args()

    if args.history is not None:
        show_history(None if args.history == -1 else args.history)
        if not args.input and not args.repeat: sys.exit(0)

    if args.repeat:
        h = get_history()
        if 1 <= args.repeat <= len(h):
            t = h[args.repeat - 1]
            if build_project(t['input'], t['bat'], t['output']): save_to_history(t)
            sys.exit(0)
        else:
            print("Invalid index."); sys.exit(1)

    if args.input and args.bat and args.output:
        if build_project(args.input, args.bat, args.output):
            save_to_history(vars(args))
    elif args.history is None:
        parser.print_help()

if __name__ == "__main__":
    main()