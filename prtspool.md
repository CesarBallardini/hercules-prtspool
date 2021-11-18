# PDF Printing from JES2


## Background

There are several ways to view job output on an MVS system. The two most common (that are available to Turnkey users) are the "hardcopy" output, which is an emulated 1403 printer dumping its contents to a file on the host file system, or interactive output viewing of held output via RPF (option 3.6).

Both have their pros and cons. Since the print to file functionality prints all output to a single file, a file can quickly become unmanageable as it grows to several megabytes in size with dozens or hundreds of jobs worth of output. Carriage control characters are also not converted which can make the output appear mangled when viewing in a standard text editor. With the RPF output viewer, only a screen's worth of text can be displayed at a time and with large jobs, it can become overwhelming.

Hercules provides another alternative in the form of print to pipe. The concept of pipes is the same as with most Linux/Unix systems. When the emulated printer has something to print, instead of writing to a file, it will redirect its output into the standard in (stdin) pipe of another program which is specified by the user. This allows virtually limitless possibilities for job output post-processing. In this example, I demonstrate how to convert raw text output to PDFs suitable for viewing or printing, but some other possibilities include e-mailing JES2 output to a user or automatically printing all output like a real mainframe printer.

Due to the nature of how the program works, it is highly dependent on the output formatting from JES2. Since I use the Turnkey system and it's a popular choice for many users, I chose to base it off that. The source code for the program and the scripts is provided, so with a little bit of C knowledge, it should be easily adaptable for other JES2 setups. prtspool was written and most heavily tested on Linux, but other users have reported success running it on Windows with Cygwin's gcc. I have not tested any other C compilers on Windows. I have also included a Universal binary for Mac OS X. I was able to do some basic testing of the PowerPC binary, but the Intel binary is untested since I do not have an Intel Mac to test it with.

## prtspool Architecture

The base program I wrote to handle this is called prtspool. It is a simple C program that provides the most basic functionality needed. All it does is split individual jobs into separate files instead of having one enormous text file with all the jobs. It can optionally call a post-processing command to process the job. It is specified in the config file as the receiveing end of the printer output.

It is invoked with `prtspool MSGCLASS OUTPUT_DIRECTORY [COMMAND]` where `MSGCLASS` is the JES2 MSGCLASS that prtspool scans for, output directory is the directory to dump output, and COMMAND is a command to call after processing a job. When the command is called, prtspool passes the MSGCLASS and output directory as parameters to the command. prtspool has no notion of JES2 job numbers, so it begins by naming the first job output job-1.txt and incrementing the number for each job. One of the major benefits of this architecture is the dynamic nature of it. In my case, I have a separate bash shell script that does all the post-processing, which allows me to customize the formatting on the fly without recompiling prtspool or having to touch anything on MVS.

##Sample Usage

The first thing that needs to be changed is the Hercules configuration file. The print to pipe syntax for 1403 devices is `DEVNUM 1403 |program`. In the case of printer 00E on the Turnkey system, the updated configuration for prtspool would look like this:

```
000E 1403 "|prtspool A /home/hercules/output/ prtpdf"
```


This says to pipe all output on device `00E` to the program `prtspool` (which must be in your path), scan for MSGCLASS A, put the output text files in `/home/hercules/output/`, and then call `prtpdf` (also must be in your path) to perform post-processing. As I said above, `prtspool` is a fairly uninteresting program and this is all the configuration needed to get it going. The interesting stuff happens in the `prtpdf`  shell script.

```bash
#!/bin/bash
if [ "$#" != 2 ]
then
  exit
fi
cd $2
for i in *.txt
do
  OUTPUT=`grep -m 1 JOB $i | cut -c15-24 | sed 's/\s//g'`
  enscript --quiet --no-header --font=Courier-Bold@8/8 --truncate-lines --landscape --margins=25:25:40:40 -p - ${i} | ps2pdf14 - ${OUTPUT}.pdf
  rm $i
done
```

At this point, when `prtpdf` is called, the invocation is equivalent to `prtpdf A /home/hercules/output/`. From bash's point of view, `$1=A and $2=/home/hercules/output/`

To provide a quick explanation on the script, the first conditional makes sure the script was called with two parameters to prevent accidental invocation from the shell. `cd $2` changes into the output directory. It then processes all `.txt` files in the directory (so make sure the directory is exclusively used for `prtspool  output). The `OUTPUT` variable uses several Linux utilities to extract the job number from the text file, but explaining the syntax is beyond the scope of this document. `enscript  is a tool which will convert plain text to Postscript, HTML, and several other formats. In this case I have `enscript` using Courier to preserve monospace formatting and I also have it use landscape orientation. `-p -` means to print to stdout. In this case it is essential since I still need to convert the Postscript to a PDF, but if omitted, it will send the output to your default printer, which may be desired in some cases. Finally the Ghostscript `ps2pdf14` script takes input from stdin and produces a PDF named after the job number. As with the OUTPUT line, see the enscript and Ghostscript manpages for more information about the programs and options.

This is just one of many possible scenarios with `prtspool`. The shell script can be changed at any point while Hercules, MVS, and `prtspool` are running.

## Download

`prtspool` is distributed with source code and binaries for Linux, Mac OS X, and Windows.

prtspool ([tar.gz, 16KB](files/prtspool.tar.gz) | [zip, 19KB](files/prtspool.zip))

The file contents are identical. Included is a 32-bit Linux binary (i386), a 64-bit Linux binary (amd64/x86_64), a Universal i386/ppc binary for Mac OS X, a 32-bit Windows binary (requires Cygwin), the prtpdf example shell script, source code, and some additional technical documentation. enscript and Ghostscript are required for the prtpdf script.

## Update (12/2/07)

Due to a request, I have provided a native Windows binary built with MinGW that does not require Cygwin. Only the Windows binary is included. The source code, extra documentation, and other binaries are still available above.

`prtspool-win32` ([zip, 6KB](files/prtspool-win32.zip))

## Additional Resources

* [enscript](https://www.gnu.org/software/enscript/)
* [Ghostscript](http://pages.cs.wisc.edu/~ghost/)
* [Cygwin](http://www.cygwin.com/)


## Disclaimers and Warnings

`prtspool` runs with the same privilege as Hercules, which may be root in some setups. As far as I can tell there are no potential vulnerabilities in prtspool, but I cannot guarantee it. I have provided the source code and encourage anyone who finds any potential vulnerabilities to notify me so I can update the program.

## Nota final

He copiado la página Web y recodificado en Markdown, para beneficio de quienes usen el repo Github. Los archivos de descarga están en el directorio `files/`en este repo.
El archivo `.tar.gz` se desempaquetó y forma el directorio `prtsppol/`. Se renombró el binario de MS Windows que requiere Cygwin a `prtspool/bin/win32/prtspool-cygwin.exe` y el binario que no requiere Cygwin se llama `prtspool/bin/win32/prtspool.exe`


