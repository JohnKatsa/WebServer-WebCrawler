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

# for every page
for dir in */; do
  cd $dir
  #echo $dir
  for file in *; do
    #1)
    k=$(( 2+RANDOM%(NUMOFLINES-2002) ))
    #echo $k
    #2)
    m=$(( 1001+RANDOM%999 ))
    #echo $m
    #3)
    f=$(($(($4/$((2))))+1))
    #echo $f
    FILES=$(ls -I $file | shuf | head -n $(($f+2)))
    #4)
    q=$(($(($3/$((2))))+1))
    FILES2=$(ls ../ -I $dir -p -R | grep -v / | shuf | head -n $(($q+2)))
    #echo $q

    # write data and links now in file $file
    echo -e "<!DOCTYPE html>\n<html>\n\t<body>\n" >> $file

    TOTAL=("$FILES $FILES2")
    arr=($TOTAL)

    let "l = $m/($f+$q)"
    txt=$(sed -n "$k,$l"p ../../$2)
    echo $txt >> $file

    printf "Adding link to $file"

    for ((i=1;i<=($f+$q);i++)); do
			let "start=$k+($i-1)*$m/($f+$q)"
			let "end=$start + $m/($f+$q)"
      awk -v s=$start -v e=$end 'NR>=s&&NR<=e' "../../$2" >> $file
      let "j=i-1"
      arr[$j]=$(find ../ -name "${arr[$j]}")
      echo "<a href="${arr[$j]}"> Link$i </a>" >> $file

      # check for incoming links
      for c in `seq 0 $incoming`; do
        if [ "${incomingnamesarr[$c]}" == "${arr[$c]}" ]; then
          let "incomingarr[$c] = 1"
        fi
      done

    done

    echo -e "\t<body>\n</html>\n" >> $file
    echo -e "\e[1;32m\t[ 0K! ]\e[0m"

  done

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
