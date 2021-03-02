#!/usr/bin/perl

open($plotdatafile, ">", "data.dict");
open($tabledatafile, ">", "data.tex");

@files = <*.log>;

print $plotdatafile "{\n";
foreach $file (@files) {
    open $fh, "<", $file;
    $mode = "none";
    while (<$fh>) {
        if (m/(.*) TASKS, (.*) PROMISES, (.*) WAITS/) {
            $tasks = $1;
            $promises = $2;
            $waits = $3;
        }
        if (m/Test Name: (.*)/) {
            print $plotdatafile "    '" . $1 . "': {\n";
            $name = $1;
        }
        if (m/((Baseline)|(Verified))/) {
            $mode = $1;
        }
        if (m/(.*) SECONDS \(data: (.*)/) {
            print $plotdatafile "        '" . $mode . "': {\n";
            print $plotdatafile "            'times': " . $2 . ",\n";
            print $plotdatafile "        },\n";
            if ($mode eq "Baseline") {
                $basetime = $1;
            } else {
                $verifiedtime = $1;
            }
        }
        if (m/(.*) AVG KBYTES/) {
            if ($mode eq "Baseline") {
                $basemem = $1;
            } else {
                $verifiedmem = $1;
            }
        }
    }

    print $plotdatafile "    },\n\n";

    printf $tabledatafile "\\addRow{%s}{%s}{%s}{%s}{%s}{%s}{%s}{%s}\n",
        $name, $basetime, $verifiedtime, $basemem, $verifiedmem, $tasks, $promises, $waits;
}
print $plotdatafile "}\n";
