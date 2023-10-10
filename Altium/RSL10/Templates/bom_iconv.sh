for file in bom*.csv; do
    enc=$(file -bi "$file")
    enc=${enc##*=}
    if [[ $enc == "unknown-8bit" || $enc == "cp1251" ]]; then
        iconv -f cp1251 -t utf-8 "$file" > temp.csv
        mv -f temp.csv "$file"
    fi
done