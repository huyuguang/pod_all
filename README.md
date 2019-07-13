# pod_all

1，拉libsnark代码  
git submodule init && git submodule update

2，拉libsnark的依赖  
cd depends/libsnark  
git submodule init && git submodule update

3，编译libsnark（使用mcl_bn128，也可以使用-DCURVE=BN128和-DCURVE=ALT_BN128，但是pod_core,pod_setup的定义也需要同步修改）  
mkdir build  
cd build  
如果是linux：
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 ..  
如果是osx:
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=OFF ..  
make  
make install  
这会在depends目录下多一个install目录。  

4，编译pod_core，pod_setup  
cd pod_core  
make  

5，运行pod_setup  
不带参数运行会在默认目录产生pk,vk。  

6，运行pod_publish发布一个文件  
-m table -f test100000.csv -o table_data -t csv -k 0 1  

6，运行pod_core  
-m table -a atomic_swap_pod_vc -p table_data -o table_output --demand_ranges 1-10  