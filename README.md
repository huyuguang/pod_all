# pod_all

1，拉libsnark代码  
git submodule init && git submodule update

2，拉libsnark的依赖  
cd depends/libsnark  
git submodule init && git submodule update

3，编译libsnark  
mkdir build  
cd build  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DWITH_PROCPS=OFF ..  
make  
make install  
这会在depends目录下多一个install目录。

4，编译pod_core，pod_setup  
cd pod_core  
make  

5，运行pod_setup  
注意pod_setup的语义已经被改变了。  
以前的pod_setup是产生一组u。这部分代码已经移入了pod_core。  
pod_core会自动在没有ecc_pub.bin的时候创建。  
pod_setup现在是zk trust setup。  
不带参数运行会在默认目录产生pk,vk。  

6，运行pod_core
