#!/bin/bash

argc=$#
required=2
start="START"
stop="STOP"

if [ $argc -ne $required ]
then
	echo "ERROR Skrypt potrzebuje 2 parametrow"
	exit 1
fi

if [ -x $1 ] && [ -d $2 ]
then
	prog=$1
	dir=$2
elif [ -x $2 ] && [ -d $1 ]
then
	prog=$2
	dir=$1
else
	echo "ERROR Musisz podac jeden plik wykonywalny i jeden katalog"
	exit 1
fi

first=`find $dir -type f -exec grep -il "START" {} \;`

temp1=`basename $0`
temp2=`basename $1`
TMPFILE=`mktemp -q /tmp/${temp1}.XXXXXX`
TMPFILE2=`mktemp -q /tmp/${temp2}.XXXXXX`
RESULT=`mktemp -q /tmp/a.XXXXXX`
cat "$first" >$TMPFILE2
grep -iv "START" $TMPFILE2 | grep -iv "STOP" | grep -iv "FILE" >$TMPFILE
<$TMPFILE ./$prog 1> $RESULT

PART2=`tail -n 1 "$first"`

if [ "$PART2" == "$stop" ]; then
	cat $RESULT
	exit 0
else
	FILE=`echo $PART2 | awk '{for (i=2; i<NF; i++) printf $i " "; print $NF}'`
	current="$dir/$FILE"
fi

i=1
while [ $i = 1 ]; do
	
	cat $RESULT > $TMPFILE2
	cat "$current" >$TMPFILE
	grep -iv "STOP" $TMPFILE | grep -iv "FILE" >>$TMPFILE2
	<$TMPFILE2 ./$prog 1> $RESULT

	
	PART2=`tail -n 1 "$current"`
	if [ "$PART2" == "$stop" ]; then
		i=0
		cat $RESULT
		rm -f $TMPFILE
		rm -f $TMPFILE2
		rm -f $RESULT
		exit 0
	else
		FILE=`echo $PART2 | awk '{for (i=2; i<NF; i++) printf $i " "; print $NF}'`
		current="$dir/$FILE"
	fi
done
	


