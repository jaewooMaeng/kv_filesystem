File System for KVSSD

참고 자료:

해당 깃헙
https://github.com/OpenMPDK/KVSSD/tree/master/PDK/driver/PCIe/kernel_driver/kernel_v4.15.18-041518-ubuntu-18_04

system 관련 블로그
https://systemdesigner.tistory.com/32

partition 관련 블로그
https://kimhyun2017.tistory.com/21

전반적인 파일시스템에 대한 블로그
http://egloos.zum.com/rousalome/v/10003473
https://jiming.tistory.com/127

모듈적재에 대한 블로그
https://www.joinc.co.kr/w/Site/Embedded/Documents/WritingDeviceDriversInLinux

ioctl 에 대한 정리
https://richong.tistory.com/254

kernel programming 강좌
https://www.youtube.com/watch?v=XUlbYRFFYf0&list=PLZ4EgN7ZCzJx2DRXTRUXRrB2njWnx1kA2

가상머신
https://teamlab.github.io/jekyllDecent/blog/tutorials/%EA%B0%80%EC%83%81%EB%A8%B8%EC%8B%A0(VirtualBox)%EC%9D%84-%EC%9D%B4%EC%9A%A9%ED%95%98%EC%97%AC-%EB%A6%AC%EB%88%85%EC%8A%A4-%EC%8B%A4%EC%8A%B5-%ED%99%98%EA%B2%BD-%EB%A7%8C%EB%93%A4%EA%B8%B0

linux 파일구조
https://webdir.tistory.com/101

바닐라커널설치
https://ioswift.tistory.com/494
https://www.youtube.com/watch?v=Z4fwmWRAppE
http://veithen.io/2013/12/19/ubuntu-vanilla-kernel.html
https://brunch.co.kr/@hajunho/328

makefile작성법
https://www.tuwlab.com/ece/27193

dummyfs 작성 설명
https://www.youtube.com/watch?v=sLR17lUjTpc

system 쪽 블로그
https://sysplay.in/blog/linux-device-drivers/2014/06/module-interactions/
https://www.linux.co.kr/home2/search/index2.php?keyword=Step+by+Step+%C4%BF%B3%CE+%C7%C1%B7%CE%B1%D7%B7%A1%B9%D6+%B0%AD%C1%C2
https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part2.html

super_block 등 설명
http://egloos.zum.com/rousalome/v/9994365

file : 이름이 붙은 byte의 집합
 -> inode # : pid처럼 해당 file의 metadata를 구분할 수 있는 번호

file system components
-> file contents(data)
-> file attributes(metadata / inode)
-> file name(path)

sector : disk가 나뉘어져 있는 단위 (512B)
block : file system이 실제로 할당하는 단위 (4KB)

partition : 하나의 물리적인 디스크를 여러 개의 논리적 디스크로 나눈 것
-> windows 의 C:\ (C가 하나의 disk)
-> linux 의 dev/sda1, dev/sda2, dev/sdb1 … (sda가 하나의 disk)

mount : 특정 디렉토리에 파일시스템을 탑재하는 것


register_chrdev : chrdev 배열 하나를 할당 받아 문자장치를 등록하는 함수(dev의 operation도 여기서 연결)
-> 해당 driver와 장치를 연결

GFP_KERNEL option : 메모리 할당이 항상 성공하도록 하는 (kmalloc) 옵션. 충분하지 않으면 대기하도록 만들 수 있다. kernel 영역에서 많이 사용되는 것 같다.

KDD 사용하는 방법
ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

3.1.3 Kernel Device Driver 
To use the kernel device driver (KDD), the user needs to compile and install NVMe modules for KV SSD, and compile the SNI A KV API with the KDD option. 
1. Compile and install NVMe modules for KV SSD.  1) cd KVSSD/PDK/driver/PCIe/kernel_driver/kernel_v<version>/ 2) make clean 3) make all 4) sudo ./re_insmod.sh  5) More details: KVSSD/PDK/driver/PCIe/kernel_driver/README  
2. Compile SNIA KV API with KDD option        1) cd KVSSD/PDK/core
3.       2) mkdir build && cd build
4.  
• If build directory already existed, all files in the directory should be deleted. 
3) cmake -DWITH_KDD=ON ../ 4) make -j24 5) kvapi library(libkvapi.so) and test binaries(sample_code_async and 
         sample_code_sync) are at: build/
      6) More details: KVSSD/PDK/core/README
3. Sample code test 
• Users must run sample codes in root privileges (users should be root or use sudo to run sample codes). 
• ./sample_code_sync -h to get usage 
• sudo ./sample_code_sync -d device_path [-n num_ios] [-o op_type] [-k klen] [-v vlen] [-t threads] 
 • sudo ./sample_code_async -d device_path [-n num_ios] [-q queue_depth] [-o op_type] [-k klen] [-v vlen] 
• Write 1000 key-value pairs of key size 16-byte and value size 4096-byte to /dev/nvme0n1 with queue depth 64 
./sample_code_async -d /dev/nvme0n1 -n 1000 -q 64 -o 1 -k 16 -v 4096 

ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

proc의 과정
do_mount() -> VFS의 read_super() -> proc_read_super() -> iget()(이를 통해 inode에 접근)

cmd 명령의 작성 매크로 함수

_IO        : 부가적인 데이터가 없는 명령을 만드는 매크로
_IOR      : 데이터를 읽어오기 위한 명령을 작성
_IOW     : 데이터를 써 넣기 위한 명령을 작성
_IOWR   : 디바이스 드라이버에서 읽고 쓰기위한 명령을 작성하는 매크로 

aio = async io

case문 -> nvme_user_kv_cmd

linux_nvme.h -> nvme_opcode
에 종류 나와있음

kernel build 시에 configuration 에서 make menuconfig가 가장 일반적, cp 로 이전 kernel에서 쓰던 설정을 그대로 가지고 와서 사용할 수도 있다.

* file system의 구조
1. task_struct 에 file 이라는 변수가 있음
2. file -> file_struct 를 가리키고 file_struct 에는 fd_array 가 존재(= fd, file descriptor)
3. fd_array 는 file 자료구조에 대한 포인터의 배열 - 파일 테이블(f_dentry, f_pos, f_op 등을 가리킨다)
4. f_dentry 는 inode 를 가리키는 포인터(실제로는 dentry 자료구조가 가리킨다)
-> file 자료구조의 f_op 는 file operations 를 가리킨다

f_op vs i_op - f_op 다양한 파일에 대한 연산 / i_op 다양한 파일시스템을 지원하기 위한 inode 연산

f_op 는 include/linux/fs.h 에 저장이 되어있다. (이는 filp_open() 의 함수가 등록한다)
-> 각 fs 에 대한 각각의 Implementation 은 fs 아래 fs/ext4/file.c 와 같이 각 폴더 안에 재정의 되어있다.

sys_read() 의 경우 파일의 유형에 따라서(f_op 을 보고) 맞는 read 함수를 부른다 (specific file layer)
-> 정규 파일의 경우 generic_read_file() -> 이는 여러 파일 시스템을 지원한다
-> inode_operations 구조에서 inode 와 관련된 연산들을 가지고  있고 이는 파일시스템에 고유한 연산들이다.

파일시스템을 작성하는 것
- super_block 을 작성
- super_operation 들을 작성(read_super, read_inode)
- f_op 작성
- i_op 작성

alloc_inode(s_op)와 create(i_op)의 차이가 무엇일까?
-> alloc은 Memory cache에 inode_cache를 할당, create는 inode를 생성
->
* struct file_system_type – contains functions to operate on the super block
* struct super_operations – contains functions to operate on the inodes
* struct inode_operations – contains functions to operate on the directory entries
* struct file_operations – contains functions to operate on the file data (through page cache)
* struct address_space_operations – contains page cache operations for the file data

file_system_type - ok
super_operations - (alloc), destroy - alloc을 등록해도 괜찮을 것 같다.
super_block

용어 정리:

kernel 모르는 용어 사전

* linux 배포판 vs linux kernel
* debian vs ubuntu
* TCP/IP, SLIP/PPP, IPX, Ethernet
* configure - 소스파일에 대한 환경설정을 해주는 명령어 
	-> 환경에 맞게 makefile을 생성해준다
	-> # ./configure --prefix = /usr/local/mysql 하게 되면 어떤 파일을 /usr/local/mysql 이라는 곳에 설치 하겠다는 뜻.
* make vs make install - make는 설치파일 생성, make install은 그 설치파일을 가지고 설치(필요한 자리에 필요한 파일 복사 등)
* nouveau - nvdia 등의 그래픽카드 등을 가동하기 위한 오픈소스(무료) 드라이버이다. 우분투를 설치하면 기본적으로 딸려나온다.
	-> 이를 사용하지 않고 nvdia 드라이버를 사용할 수도 있다.
* 디렉토리 vs 폴더 - linux에서는 directory, windows에서는 folder -> linux는 / windows는 \ 로 구분
* FSSTND(LINUX FILE System Standard)(리눅스 파일 시스템 표준) - etc, usr 등 파일 시스템의 표준 형태
* GNU - 운영체제의 하나이자 소프트웨어 모음집, linux kernel과 결합된 형태가 우리가 흔히 아는 LINUX 운영체제이다.
* 부트스트랩 프로그램 - 전원을 켜거나 재부팅할 때 적재되는 프로그램(운영체제 적재하고 실행시킨다)
* GRUB - GNU 프로젝트의 부트로더로 다수의 운영체제의 커널을 불러올 수 있다.
* SSH(secure shell protocol) - 보안적으로 안적하게 통신을 하기 위한 프로토콜
* x 윈도우 - LINUX/UNIX 기반 시스템에서 cmd처럼 사용하지 않고 GUI를 사용할 수 있게 해주는 프로그램(그래픽 환경 기반 시스템 소프트웨어)
* apt-get - ubuntu난 linux 의 패키지 관리 -> apt-get install로 설치 가능(pip과 비슷한듯)
* emulator - 어떤 시스템인척 할 수 있는 프로그램이나 장치이다.
* 파이프 - process 간 통신의 일종으로 일방향성 통신이다.
* entry/dentry - 해당 디렉토리가 포함하고 있는 디렉토리와 파일정보를 보유하는 구조체(dentry)
* magic number
* GPL/LGPL/BSD/MIT license - 각각 소프트웨어 라이센스이다. GPL의 경우 사용하였다는 것을 표기 및 소스코드를 공개해야한다. LGPL은 사용했다는 것만 표기하면 된다. BSD, MIT는 마음대로 해도 된다.
* slab/slub(슬랩/슬럽, kmem_cache) - 미리 자주 등장하는 할당을 해놓은 것 -> 해당 주소를 가리킨다.
* 십볼릭 링크 - 윈도우의 바로가기와 비슷한 개념으로 파일에 대해 ln -s [원본] [카피] 와 같이 생성할 수 있다.
