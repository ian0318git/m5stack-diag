	# 使用 AlmaLinux 8 作為基底 (RHEL 8 的免費替代版)
	FROM almalinux:8
	
	# 安裝開發者工具組 (內含 gcc, make, binutils 等)
	RUN yum groupinstall -y "Development Tools"
	
	# 在 Build Image 時就建立好軟連結
	RUN ln -s /usr/bin/gcc /usr/bin/gcc.c4.7.0-p5.x86_64

	# 如果你需要特定的舊版相容性，可以加裝這行
	# RUN yum install -y gcc-toolset-9
	
	# 設定工作目錄
	WORKDIR /usr/src/app
	CMD ["/bin/bash"]
