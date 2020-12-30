# 소켓
소켓은 인터넷을 사용하기 위한 API(Application Programming Interface)로, 물데네전세표응(OSI 7계층) 중에서 전송 계층(Transport Layer)에서 사용됨.  
소켓은 한 애플리케이션 프로세스가 다른 애플리케이션으로 메세지를 보내거나 받을 수 있는 host-local, application-created, OS-controlled 인터페이스임.  
Local Socket Address는 발신자 집주소, Remote Socket Address는 수신자 집주소, Protocol은 편지 봉투로 생각하면 됨.  
이때 Address를 통해 발신자나 수신자를 반드시 식별 가능해야하며, Address는 IP 주소를 사용한다.
- IPv4: 32bit, X.X.X.X (X:8bit)
- IPV6: 128bit, Y:Y:Y:Y:Y:Y:Y:Y (Y:16bit)

프로세스의 주소는 Port로 구별한다. (0~65535 사이의 숫자)  

- 0 ~ 1023 사이의 포트번호는 시스템 실행에 사용되는 번호라서 소켓을 사용할떄는 1024 ~ 65535 사이의 포트를 사용하는게 좋다.
- 80번 포트는 HTTP용, 25번 포트는 SMTP, 21번 포트는 FTP 용도로 사용됨.

소켓의 프로토콜은 UDP와 TCP가 있다.

# 프로젝트 진행 기간
2017.10.13. ~ 2017.11.03. (3주)
