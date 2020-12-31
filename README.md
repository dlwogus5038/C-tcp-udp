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

# Task1 : UDP Programming
- Modify the UDP Server & Client
- (Option) How to write a chat proramming (two clients chat with each other) with UDP?
- (Option) Can we use the DUP to transfer a file? If so, how?

# Task2 : Implementing FTP
## FTP Server
 - Berkeley Socket API와 C언어를 사용하여 FTP 서버 구현.
 - Port와 Directory 두가지 매개변수를 받아야 함.
 - USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, LIST, RMD 명령어 구현 필요.
 - 잘못된 입력을 합리적으로 처리하고 방어 가능한 오류 코드를 생성해야함.
 - 여러 클라이언트의 연결을 지원.
 - FTP 기능이 구현돼있는 특정 라이브러리 사용 금지.
 - (Option) How to transfer large files without blocking the server?
 ## FTP Client
  - 원하는 언어로 작성
  - USER, PASS, TYPE, RETR, STOR, QUIT, PORT, PASV 명령어를 구현해야 하고, 서버에 로그인하는 기능과 파일을 업로드/다운로드 하는 기능 구현 필요.
  - 디렉토리 생성, 변경, 나열, 제거 작업 지원.
  
  # 명령어 설명
  
  ## USER  
  로그인 ID 입력 명령.
  클라이언트 측에서 "USER anonymous" 라고 입력하면 "331 Guest login ok, send yout password." 라는 메세지 전달해주기.
  
  ## PASS
  로그인 PW 입력 명령.
  클라이언트측에서 "PASS XXXX" 라고 입력하면 "230 Login successful" 메세지 전달.
  
  ## PORT
  FTP는 파일이나 디렉토리를 전송할때 또다른 연결을 생성해서 전송을 진행함. 클라이언트가 연결을 요청할때 사용하는 명령어.  
  PORT 요청은 h1,h2,h3,h4,p1,p2 6가지 매개변수를 함께 서버로 보냄.  
  클라이언트가 TCP 포트 p1*256+p2의 연결을 IP 주소 h1.h2.h3.h4에서 수신 대기 중임을 의미.  
  서버는 PORT 명령어를 받자마자 연결하는게 아니라, 클라이언트가 RETR 명령어를 보내고 서버가 initial mark를 보낸 후에 연결을 시도함. 
  연결 성공시 code 200을 보내고, 연결이 실패하면 code 425를 보냄.
  
  ## PASV
  PORT와 비슷한 명령어. 하지만 PASV 명령어는 PORT 명령어와 반대로 서버가 클라이언트측으로 연결 가능한 IP 주소와 PORT 번호를 전송함.  
  클라이언트가 PASV 명령어를 보내면 서버는 "227 Entering Passive Mode (166,111,80,233,128,2)" 와 같은 메세지 전송.  
  해당 메세지를 보내기 전에 서버는 해당 포트에 소켓을 열어서 연결을 대기하고 있어야함.  
  만약 클라이언트가 PASV 명령어를 이미 보낸 상태에서 다시 PASV 명령어를 보낼시에는 기존에 대기하고 있던 포트를 막고 기존의 연결을 끊음. 그리고 다시 새로운 연결 시도.
  PORT 명령어와 PASV 명령어는 일회용임. 한번 파일이 전송이 끝나면 다시 PORT나 PASV 명령어로 연결해서 파일을 전송해야함.
  
  ## RETR
  FTP는 파일을 전송할때 사용됨. RETR은 서버로부터 특정 파일을 다운로드 받고 싶을때 사용하는 명령어. 파일 전송은 Binary Mode로 진행됨.  
  "RETR <filename>" 같이 사용됨.
  - 전체 파일이 서버의 TCP 버퍼에 성공적으로 기록된 경우 코드 226으로 RETR 요청을 수락.  
  - TCP 연결이 설정되지 않은 경우 코드 425로 RET 요청을 거부.  
  - TCP 연결이 설정되었으나 클라이언트 또는 네트워크 오류로 중단된 경우 코드 426으로 RET 요청을 거부.  
  - 서버에서 디스크에서 파일을 읽는 데 문제가 있는 경우 코드 451 또는 551을 사용하여 RET 요청을 거부. 서버는 이러한 경우 각각 데이터 연결을 닫음.  
  클라이언트는 서버가 파일을 전송 완료하고 연결을 닫을때까지 아무런 response를 서버로부터 받지 못함.
  
  ## STOR
  STOR은 서버에 특정 파일을 저장하고 싶을 때 사용하는 명령어.  
  RETR과 STOR 명령을 수행하고 있을때는 해당 클라이언트로 부터 오는 명령어를 모두 무시해야함. (ABOR / QUIT 같은 명령어는 가능)  
  해당 클라이언트로 부터 오는 명령만 전부 무시해야하는거고, 새로운 클라이언트와의 연결이나 파일 전송은 정상적으로 수행돼야함.
  
  ## SYST
  해당 명령을 받으면 "215 UNIX Type: L8" 메세지를 클라이언트로 전달
  
  ## TYPE
  "TYPE I" 라는 명령을 받으면 "200 Type set to I" 라는 메세지를 클라이언트로 전달.
  
  ## QUIT
  QUIT 명령을 받으면 해당 클라이언트를 로그아웃 시켜야함. 해당 클라이언트가 연결한 파일 전송용 연결들도 전부 끊어야함.
  
  ## ABOR
  Abort는 QUIT과 비슷한 명령.
  
## EXAMPLE

> 1. Client connects to the server (ftp.ssast.org)
> 2. Server responds with an initial message
> “220 ftp.ssast.org FTP server ready.”
> 3. Client then attempts to log in by sending
> “USER anonymous”
> 4. Server parses the argument, determines it is open and requests the client for the password with the following response
> “331 Guest login ok, send your complete e-mail address as password.”
> 5. Client responds with the email address as the password
> “PASS dangfan@163.com”
> 6. Server determines that the username and password are acceptable. It logs the client in and displays the welcome message. Notice that only the last line of the welcome message contains a valid mark as explained in section 3.5
> > 230-  
> > 230-Welcome to  
> > 230- School of Software  
> > 230- FTP Archives at ftp.ssast.org  
> > 230-  
> > 230-This site is provided as a public service by School of  
> > 230-Software. Use in violation of any applicable laws is strictly  
> > 230-prohibited. We make no guarantees, explicit or implicit, about the  
> > 230-contents of this site. Use at your own risk.  
> > 230-  
> > 230 Guest login ok, access restrictions apply.  
> 7. Client tries to determine the servers operating system and type settings by sending a SYST command
> “SYST”
> 8. Server responds with its SYST settings
> “215 UNIX Type: L8”
> 9. Client decides to set the TYPE to binary (type I)
> “TYPE I”
> 10. Server responds that the operation was successful
> “200 Type set to I.”
> 11. Client sends the PORT command to the server
> “PORT 166,111,80,233,128,79”
> 12. Server responds that it acknowledges the PORT command
> “200 PORT command successful.”
> 13. Client attempts to retrieve a file (robots.txt)
> “RETR robots.txt”
> 14. Server opens a binary connection to the IP address and port number specified by the earlier PORT command
> “50 Opening BINARY mode data connection for robots.txt (26 bytes).”
> 15. Server tells the client that the transfer is complete
> “226 Transfer complete.”
> 16. Client decides to tell the server to use PASV mode instead
> “PASV”
> 17. Server responds with the IP address and port number for the client to connect to
> “227 Entering Passive Mode (166,111,80,233,102,109)”
> 18. Client retrieves the same file again (robots.txt) “RETR robots.txt”
> 19. Server accepts a connection from the client to the IP address and port number specified by the PASV command. It sends the file over in binary mode. Note that the messages that the server returns are identical in both PORT and PASV mode.
> “150 Opening BINARY mode data connection for robots.txt (26 bytes).”
> 20. Server tells the client that the transfer is complete
> “226 Transfer complete.”
> 21. Client decides to logout
> “QUIT”
> 22. Server logs the client out and displays some statistics about the ftp connection
> > 221-You have transferred 52 bytes in 2 files.  
> > 221-Total traffic for this session was 1975 bytes in 2 transfers.  
> > 221-Thank you for using the FTP service on ftp.ssast.org.  
> > 221 Goodbye.  
  
# 얻은점
C언어로 UDP 채팅 구현, FTP 서버 구축하는 법에 대해 배움. 기술적으로는 소켓을 이용해 통신하는법, TCP와 UDP의 연결 방식, 멀티스레딩을 이용한 병렬처리에 대해 배울 수 있었음.
