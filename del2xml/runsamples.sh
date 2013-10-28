sed 's/ *, */$/g' < sample.csv > sample2.csv
sed 's/ *, */Z/g' < sample.csv > sample3.csv

./del2xml.perl < sample.csv > sample.xml
./del2xml.perl -dollar < sample2.csv > sample2.xml
./del2xml.perl -del Z < sample3.csv > sample3.xml

diff *2.xml *3.xml > all.diff
diff sample.xml *3.xml >> all.diff
echo 'test.  wc should be zero'
wc -l all.diff
