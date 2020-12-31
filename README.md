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

# UDP
- 클라이언트와 서버 간에 바이트 그룹("데이터그램")을 최적으로 전송합니다.
- Connectionless  
수신자가 통보하지 않는 한 패킷의 수신여부를 알 수 없습니다.
- Datagrams == Groups of bytes == packets  
모든 패킷에는 Address 정보가 필요합니다.  
out-of-order
- Best-effort == unreliable
- Lightweight

# TCP
- 한 프로세스에서 다른 프로세스로 바이트 스트림을 안정적으로 전송합니다.
- Connection Oriented (연결 지향적)
- Byte-stream  
애플리케이션 계층에는 패킷화 및 주소가 필요하지 않습니다.  
In order
- 신뢰할 수 없는 네트워크에 신뢰할 수 있는 "파이프"를 구축합니다.
- 연결 유지 관리

## Byte-Stream
- 프로세스에 들어가거나 프로세스에서 나오는 일련의 문자입니다.
- Input stream: stdin(키보드), socket
- Output stream: stdout(모니터), socket

# 프로젝트 진행 기간
2017.10.13. ~ 2017.11.03. (3주)
