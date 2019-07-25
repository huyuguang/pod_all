# pod_all

Core code for https://github.com/sec-bit/zkPoD-lib  

### 获取代码
1，拉libsnark代码  
git submodule init && git submodule update

2，拉libsnark的依赖  
cd depends/libsnark  
git submodule init && git submodule update

### 编译（linux or osx）
1，编译libsnark（使用mcl_bn128，也可以使用-DCURVE=BN128和-DCURVE=ALT_BN128，但是pod_core,pod_setup的定义也需要同步修改）  
mkdir build  
cd build  

如果是linux（g++）：  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DUSE_PT_COMPRESSION=OFF -DMONTGOMERY_OUTPUT=OFF -DBINARY_OUTPUT=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=ON ..  
如果是osx（clang++）:  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=OFF -DUSE_PT_COMPRESSION=OFF -DMONTGOMERY_OUTPUT=OFF -DBINARY_OUTPUT=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=OFF ..  

make  
make install  
这会在depends目录下多一个install目录。  

2，编译pod_core，pod_setup，pod_publish（linux or osx）  
cd pod_core  
make  

cd ../pod_setup  
make  

cd ../pod_publish  
make  

编译好的代码在linux/bin目录下。  

3，运行pod_setup  
cd linux/bin  
./pod_setup  
不带参数运行会在默认目录产生pk,vk。  

4，运行pod_publish发布一个文件  
cd linux/bin  
./pod_publish -m table -f test100000.csv -o table_data -t csv -k 0 1  

5，运行pod_core  
cd linux/bin  
./pod_core -m table -a atomic_swap_pod_vc -p table_data -o table_output --demand_ranges 1-10  


### 编译 (windows + msvc2019)：  
1，首先编译libsnark，参看libsnark/msvc/README.md。  
2，然后直接用msvc2019打开pod_all.sln。  
