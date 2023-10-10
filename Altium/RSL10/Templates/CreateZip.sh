filesExt="*.G..|*TXT"
echo ${filesExt}
files=$(find|egrep ${filesExt})
nameZip=$(basename $(pwd)).zip
echo ${files}
/c/Program\ Files/7-Zip/7z a ${nameZip} ${files}