#I_PNG=8192
I_OTH=65536
I_PNG=1
#I_OTH=1
F="-n1 -n2 -n3 -n4 -n5 -n6 -n7 -n8 -n9 -n10 -n11 -n12 -n13 -b0 -b128 -b256 -b512 -b1024 -b2048"
R=64


for file in $*; do
    if echo "$file" | grep 'png$'; then
        optipng -o7 "$file"
        for r in `seq 1 $R`;do
        for s in $F;do \
            pngout -r -c3 -n$s "$file"
        done
        done
        advpng -z4 -i$I_PNG "$file"
        DeflOpt "$file"
    else
        advdef -z4 -i$I_OTH "$file"
        ln "$file" "$file"-tmp.gz && DeflOpt "$file"-tmp.gz; rm "$file"-tmp.gz
    fi
done
