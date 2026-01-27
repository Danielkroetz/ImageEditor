use strict;

my $program = "../build/ImageEditor --batch";

print "Select batch job [1-6]: ";
my $batchId = <STDIN>;
chomp($batchId);

my $infile = "./images/clean_pm3736o_gb_proc.png";
my $projectfile = "projects/imageeditor_batch".$batchId.".json";
if ( -e $projectfile ) {
 system($program." --file $infile --project $projectfile --output test".$batchId.".png");
} else {
 die "FATAL ERROR: Invalid project file '".$projectfile."': $!";
}
