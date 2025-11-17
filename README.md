🖥️ UniShell – A Hybrid Shell

UniShell is a lightweight hybrid shell that allows users to execute system commands locally using a C-based shell and remotely using a browser interface powered by Flask + TCP/LocalTunnel (Node.js).
It is designed as an educational Operating Systems project to demonstrate real OS concepts like process management, IPC, and safe remote command execution.

🚀 Project Overview

UniShell is a hybrid operating-system shell that supports local command execution (via C programs on WSL/Linux) and remote command execution using tcp(server/client model)and LocalTunnel(through Node.js), without relying on SSH or complex networking setups.
The system demonstrates core Operating System concepts including:
1. Process creation & management (fork(), execvp(), wait()).
2. File system commands & I/O handling.
3. Signal handling (SIGINT, SIGCHLD).
4. Client–server architecture using TCP client/server in C.
5. Remote access via a browser using LocalTunnel (instead of SSH ,SSL).
6. Flask/HTML interface for executing commands locally and remotely.

This project bridges theoretical OS concepts with hands-on execution, allowing users to run shell commands locally or through a secure remote link.

🧩 Key Features
✔ Local C Shell
1.Executes Linux/WSL commands
2.Supports foreground/background processes
3.Handles signals and I/O redirection
4.Built using C and Unix system calls

✔ TCP Client–Server (C)
1.tcp_server.c receives commands from remote interface
2.tcp_client.c sends commands and receives responses
3.Light, fast, and OS-concept–friendly

✔ Web Interface (Flask + HTML)
1.Simple browser-based UI (index.html)
2.Sends shell commands
3.Displays real-time output

✔ Remote Access using LocalTunnel (Node.js)
1.Creates a public HTTPS link to your local Flask server
2.No port forwarding
3.No SSH
This project works from anywhere.

🛠 Tech Stack
Component	Technology
1. Local Shell	C (WSL/Linux)
2. Remote Access	LocalTunnel (Node.js)
3. Networking	TCP sockets (C programs)
4. Backend	Python Flask
5. Frontend	HTML, CSS, JavaScript
6. Communication	JSON-based API + TCP client/server🎯 Project Goals

📡 System Workflow
1. User enters command in browser
2. Flask receives the command
3. Flask forwards command to tcp_client.c
4. tcp_client.c sends it to tcp_server.c
5. tcp_server.c executes it using the C shell
6. Output is sent back → Flask → Browser
7. LocalTunnel exposes Flask to the internet securely

🎯 Project Goals
1. Implement OS-level shell operations
2. Demonstrate process handling & IPC using C
3. Provide remote access without SSH
4. Create an educational tool for OS labs
5. Show real-time shell execution through the browser

📘 Future Enhancements
1. Command history & multi-user sessions
2. Authentication & access control
3. Secure sandboxing
4. WebSocket-based streaming
5. File upload/download via shell
