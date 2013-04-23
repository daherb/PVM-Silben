PVM-Silben
==========

Parallel Map-Reduce style program to split words in syllables.
for each word a tree of all possible partitions is returned

Configuration
-------------
All parameters are set in the consts.h header. The
frequency list of words to split is a simple file
with one word and the word count in one line separated
by tab. The syllable list is even simpler with one syllable
per line.

Output
------
The program writes a file with each line consisting of a word,
the partition tree and the word count separated by tabs

Debugging
---------
The master/slave programs write log files to /tmp. The filenames
are /tmp/silben-master.log and /tmp/silben-slaveTID.log with
TID the client task id