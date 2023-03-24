### 1. 项目背景

云上的高性能计算（ Cloud High-Performance Computing, Cloud HPC ），与本地集群相比，有多方面的区别。其中最为显著的区别，就是云资源的弹性、动态与灵活性。理论上讲，您可以轻松使用多个云计算机房的计算、存储、网络资源，您的超算集群规模可以扩展到数百、数千核心，也可能根据实际情况减小到 0 个计算核心。这种动态特点就意味着，您在一定程度上需要关心底层资源 —— 例如集群的规模、集群的数量等。

然而，资源管理对于广大的 Cloud HPC 用户来说，是比较陌生的；尤其是云资源的管理，更是无从下手。云提供海量的底层资源，但是 Cloud HPC 用户首先需要让这些资源组成具备超级计算能力的集群。这也就意味着需要一个  **构建和运行** 的过程。这个过程，对于大多数用户来说，难度非常高，原因在于它涉及到多方面的 IT 应用知识和云计算技能，包括但不限于：

- 什么是网络、虚拟私有网络、子网、网段、ACL、公有和私有 IP ……
- 什么是云主机、主机镜像、安全组、公钥私钥 ……
- 什么是云硬盘、文件存储、对象存储、挂载不同类型的存储 ……
- 什么是任务调度器、调度器如何配置和运行、超算用户如何使用和提交任务 ……
- ……

以上种种，都成为了制约 Cloud HPC 走向更广阔应用的一道门槛。目前，这些工作要么是由云厂商以自研的方式、面向 HPC 用户推出自有的 Cloud HPC 服务，要么是由第三方服务商进行集成开发之后以类 SaaS 平台的方式提供给客户。无论是哪一种方案，都旨在将客户留在各自的平台上；尽管在商业方面是完全合理而且完全值得尊重的，但是无形之中会限制最终用户对于超算的掌控权以及选择 Cloud HPC 服务的自由度。

为了打造全面开放且超级简单的 Cloud HPC 平台，让用户能够以极低的门槛开启 Cloud HPC，加速科研创新，我们打造了开源的云超算平台 HPC-NOW。这里，NOW 有两层含义：

- 现在即可开始，无需等待
- No Operation Workload，无需繁重的运行维护工作，即可在云上轻松管理您的超算集群和超算服务

希望与您共同携手打造开源、开放的 Cloud HPC 生态！

### 2. 核心组件

首先，在此鸣谢卓越、开源、面向多云的 IaC（ 基础设施即代码 ）平台 [Terraform](http://www.terraform.io)。正是 Terraform 强大的功能和良好的生态为 HPC-NOW 提供了坚实的基础，我们可以不必重复造轮子。本项目的核心工作，在于如何驱动 Terraform 来构建和管理一个或多个 Cloud HPC 集群。

本项目的核心组件如下：

- installer ：HPC-NOW 服务的安装器。主要负责服务的安装、卸载、更新三项工作。该安装器被设计为必须由管理员权限执行。
- hpcopr    ：意即 HPC Operator，是 HPC-NOW 的核心程序，也是用户需要执行的主程序。为了确保安全性和隔离性，该程序被设计为必须由专属 OS 用户 "hpc-now" 执行，其他用户，即使是管理员用户或者根用户也无法执行。由 hpcopr 管理基础设施代码，并调用 Terraform 对云资源进行全生命周期管理。
- now-crypto：辅助程序，主要作用是简单的文件加密和解密，以确保敏感信息不以明文形式存放。请注意，该程序不是严格的加解密程序，仅通过统一偏移字符的方式对文本信息进行修改，您的密文文件仍需要妥善保管。一旦密文文件泄露，他人可能通过穷举得到您的文本偏移量，从而反向偏移得到原文。
- hpcmgr    ：强大的集群内管理工具，包括集群的连接、SLURM 服务的启动、以及 HPC 软件包的自动化编译安装等
- Templates ：模板文件是 IaC（基础设施及代码）的核心要素，我们已经针对 AWS、阿里云、腾讯云 三家公有云厂商制作了专用的资源模板，后续将进一步接入 微软 Azure、GCP、华为云等三家云资源
- Scripts   ：启动脚本包含了集群各个节点启动过程的编排，包括各类必要组件的自动化安装

### 3. 构建环境需求

我们使用 C 语言和 GNU/Linux Shell 脚本进行整个平台的构建。其中，三个核心程序 installer、hpcopr、now-crypto 均为纯 C 语言编写。C 语言跨平台和偏底层的特点，使得其适合用来进行核心程序的开发。对于这三个核心程序而言，构建过程仅需要 C 语言编译器即可，在三个主流操作系统方面，需要安装的 C 语言编译器略有不同。

- Microsoft Windows：您需要安装最新版 mingw，具体请参考教程：https://blog.csdn.net/LawssssCat/article/details/103407137
- GNU/Linux：您需要安装 gcc，版本一般为 8.x.x 及以上，请从自带的软件仓库中安装，如 yum 或者 apt，示例命令：sudo yum -y install gcc
- macOS：您需要安装 clang，版本一般为 13.x.x 及以上，在 Terminal 输入 clang 之后，如果本机没有安装clang，macOS会询问是否安装，可根据提示进行自动安装

### 4. 如何构建

请从本项目的 dev 分支下载源代码至本地目录（ 例如 /home/ABC/hpc-now-dev/ ），使用 'cd' 命令切换至代码所在的本地目录之后：

- Microsoft Windows用户，请运行：gcc hpcopr-windows.c -Wall -o hpcopr.exe
- GNU/Linux用户，请运行：gcc hpcopr-linux.c -Wall -lm -o hpcopr
- macOS用户，请运行：clang hpcopr-darwin.c -Wall -o hpcopr

此外，还需以相似的方式编译 now_crypto.c，并将生成的可执行文件命名为 now-crypto.exe

分别从 hpcopr-*OS*.c 和 now_crypto.c 构建出两个可执行文件之后，请将基于 hpcopr-windows.c | hpcopr-darwin.c | hpcopr-linux.c 构建形成的可执行文件命名为 hpcopr；将 now_crypto.c 构建形成的可执行文件命名为 now-crypto.exe

### 5. 如何使用

请参阅部署手册：https://www.hpc-now.com/deploy 。请注意：如果您直接运行 hpcopr 本地安装，将会下载云上已经编译好的 now-crypto 至以下本地目录：

- Windows： C:\programdata\hpc-now\bin\now-crypto.exe
- GNU/Linux：/usr/.hpc-now/.bin/now-crypto.exe
- macOS：/Applications/.hpc-now/.bin/now-crypto.exe

您可以用自己构建的 now-crypto 替换掉安装时下载的文件，请注意文件名保持一致即可。此外，在 GNU/Linux 和 macOS 下，注意要赋予可执行权限，示例命令： 
- sudo chmod +x now-crypto

### 6. 关键目录
hpcopr 安装部署之后，将会对您的操作系统进行如下修改。具体的修改操作请阅读源代码的 check_and_install_prerequisitions 函数内容。

- 创建一个名为 hpc-now 的操作系统用户，对于 Microsoft Windows，该用户将生成初始密码 nowadmin2023~ ，并且在首次以 hpc-now 用户登录时强制要求修改；对于其他操作系统，无初始密码，您需要按照说明为该用户设置密码
- 创建工作目录，对于 Microsoft Windows，将创建 C:\hpc-now 目录作为关键工作目录，创建 C:\programdata\hpc-now 作为关键数据目录；对于 GNU/Linux，关键工作目录位于 hpc-now 的家目录下，即：/home/hpc-now，关键数据目录位于 /usr/.hpc-now ；对于 macOS，关键工作目录位于 hpc-now 的家目录下，即：/Users/hpc-now，关键数据目录位于 /Applications/.hpc-now 。

上述对操作系统的修改均不会无意或恶意破坏您的操作系统。同时，您可以随时以管理员身份执行 uninstall 操作回滚上述对操作系统的改动。请注意，uninstall 操作可能会导致您对云上的集群失去管理权，请务必按照软件说明进行确认操作。

如有任何疑问，请您阅读源代码、或者联系我们进行技术沟通。

### 7. Bug与技术沟通
欢迎随时联系 info@hpc-now.com