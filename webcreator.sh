#!/bin/bash
echo "Root Directory Name : " $1
echo "Text File Name : " $2
echo "w = " $3
echo "p = " $4

# check existence of directory
if [ ! -d $1 ]; then
  echo "Directory not found!"
fi

# check if directory is empty
if [ "$(ls -A $1)" ]; then
  printf "Warning: directory is full, purging ..."
  rm -r ./$1/*
  echo -e "\e[1;32m\t[ 0K! ]\e[0m"
fi

# check existence of file
if [ ! -f $2 ]; then
    echo "File not found!"
fi

# check number of lines in text file >= 10000
NUMOFLINES=$(wc -l < $2)
#echo "NUMOFLINES = " $NUMOFLINES
if [ $NUMOFLINES -lt 10000 ]; then
    echo "File has less than 10000 lines!"
fi

cd $1

index=0
# make w site directories (0,w-1)
DNAME="site"
FNAME="page"
let MAX=$3-1

incoming=0
incomingarr=()
incomingnamesarr=()
for i in `seq 0 $MAX`; do
  echo "Creating web site " $i " ..."
  TEMP="$DNAME$i"
  mkdir $TEMP

  cd $TEMP

  # make for each directory p files
  for j in `seq 1 $4`; do
    # check if already exists this RANDOM number in other file
    TEMP2="$FNAME$i"_"$RANDOM".html""
    until [ ! -f $TEMP2 ]; do
      TEMP2="$FNAME$i"_"$RANDOM".html""
    done

    touch $TEMP2
    printf "Creating page $(find ../ -name $TEMP2)"
    echo -e "\e[1;32m\t[ 0K! ]\e[0m"
    incomingnamesarr[$incoming]=$TEMP2
    let "incomingarr[$incoming]=0"
    let "incoming++"

    # array contains all pages from all sites
    array[$index]=$TEMP2
    let index++

  done

  cd ..
done

echo -e "\n"
let "incoming--"

#echo "index = " $index

# for every page
for dir in */; do
  cd $dir
  #echo $dir
  for file in *; do
    #1)
    k=$(($(($RANDOM%$((`expr $NUMOFLINES-2001`))))+1))
    #echo $k
    #2)
    m=$(($(($RANDOM%$((1000))))+1000))
    #echo $m
    #3)
    f=$(($(($4/$((2))))+1))
    #echo $f
    FILES=$(ls -I $file | shuf | head -n $f)
    #4)
    q=$(($(($3/$((2))))+1))
    FILES2=$(ls ../ -I $dir -p -R | grep -v / | shuf | head -n $q)
    #echo $q

    # write data and links now in file $file
    echo -e "<!DOCTYPE html>\n<html>\n\t<body>" >> $file

    #TOTAL=$(ls -I $file | shuf | head -n $f && ls ../ -I $dir -R | shuf | head -n $q)
    TOTAL="$FILES $FILES2"
    arr=($TOTAL)
    #echo "total = ", $TOTAL

    let "l = $m/($f+$q)"
    txt=$(sed -n "$k,$l"p ../../$2)
    #txt=$(sed -n -s '1,2!d' ../../$2)
    #txt=$(cat ../../$2 -n | grep " 50" -b10 -a10)
    echo $txt >> $file

    printf "Adding link to $file"

    for ((i=1;i<=($f+$q);i++)); do
			let "start=$k+($i-1)*m/($f+$q)"
			let "end=$start + $m/($f+$q)"
			#echo "start=$start end=$end"
      awk -v s=$start -v e=$end 'NR>=s&&NR<=e' "../../$2" >> $file
      #echo >> $file
      #a=$(sed -n -e "$i"p "../../$2")
      #echo "ela" $(awk -F ' ,' '$TOTAL')
      j=`expr $i - 1`
      #echo "arri = " "${arr[$j]}" " hrecehyhh"
      arr[$j]=$(find ../ -name "${arr[$j]}")
      #echo "arri = " ${arr[$j]}
      #echo "ela" "${arr[$j]}" "esdasfgd"
      echo "<a href="${arr[$j]}"> Link$i </a>" >> $file

      # check for incoming links
      for c in `seq 0 $incoming`; do
        if [ "${incomingnamesarr[$c]}" == "${arr[$c]}" ]; then
          let "incomingarr[$c] = 1"
        fi
      done

    done

    echo -e "\t<body>\n</html>" >> $file
    echo -e "\e[1;32m\t[ 0K! ]\e[0m"

  done

  #echo $FILES
  cd ..
done

# check if all pages have at least one incoming link
flag=0
for c in `seq 0 $incoming`; do
  if [ "${incomingarr[$c]}" -eq 1 ]; then
    let "flag++"
  fi
done
let "incoming++"
if [ "$flag == $incoming" ]; then
  echo -e "\nAll pages have at least one incoming link"
fi
echo "done."
