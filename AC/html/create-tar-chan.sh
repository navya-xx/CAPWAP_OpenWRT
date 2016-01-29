#!/bin/sh

# This file will create file to update freq and txpower
# create files -> update.cud, WTP/, scripts/ -> pre.sh, post.sh

#PATH_TMP=$(pwd)

mkdir /tmp/capwap-tmp
mkdir /tmp/capwap-tmp/WTP
mkdir /tmp/capwap-tmp/scripts

echo "version: 0.93.2" > /tmp/capwap-tmp/update.cud
echo "PreUpdate: pre.sh" >> /tmp/capwap-tmp/update.cud
echo "PostUpdate: post.sh" >> /tmp/capwap-tmp/update.cud

CMD="#!/bin/sh\n"
CMD="$CMD uci set wireless.@wifi-radio0[-1].channel=$1\n"
CMD="$CMD uci commit\n"
CMD="$CMD wifi\n"
CMD="$CMD touch /tmp/navya_test.file"

echo $CMD > /tmp/capwap-tmp/scripts/pre.sh
echo "#!/bin/sh" > /tmp/capwap-tmp/scripts/post.sh

chmod 755 /tmp/capwap-tmp/scripts/pre.sh
cd /tmp/capwap-tmp/
tar -zcvf capwap-tmp.tar.gz *
mv capwap-tmp.tar.gz /tmp/
rm -r /tmp/capwap-tmp/
