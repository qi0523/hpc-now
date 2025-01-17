# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *zlib-1.2.13* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo | grep "processor" | wc -l`

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_cache="/hpc_apps/.cache/"
  envmod_root="/hpc_apps/envmod/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
  envmod_root="/hpc_apps/envmod/${current_user}_env/"
fi

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the binaries and libraries ..."
  rm -rf ${app_root}zlib-1.2.13
  rm -rf ${envmod_root}zlib-1.2.13
  if [ $current_user = 'root' ]; then
    sed -i '/< zlib >/d' ${public_app_registry}
  else
    sed -e "/< zlib > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] App removed successfully."
  exit 0
fi

mkdir -p $app_cache
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -f ${app_cache}zlib-1.2.13.tar.gz ]; then
  wget ${url_pkgs}zlib-1.2.13.tar.gz -O ${app_cache}zlib-1.2.13.tar.gz -o ${2}
fi
tar zxf ${app_cache}zlib-1.2.13.tar.gz -C ${app_cache} >> /dev/null 2>&1

echo -e "[ STEP 1 ] Building zlib-1.2.13 ... This step usually takes seconds."
cd ${app_cache}zlib-1.2.13
./configure --prefix=${app_root}zlib-1.2.13 >> ${2} 2>&1
make -j${num_processors} >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build zlib-1.2.13. Please check the log file for more details. Exit now."
  exit 5
fi
make install >> ${2}
echo -e "#%Module1.0\nprepend-path LD_LIBRARY_PATH ${app_root}zlib-1.2.13/lib\nprepend-path C_INCLUDE_PATH ${app_root}zlib-1.2.13/include" > ${envmod_root}zlib-1.2.13
if [ $current_user = 'root' ]; then
  echo -e "< zlib >" >> $public_app_registry
else
  echo -e "< zlib > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] zlib-1.2.13 has been successfully installed."