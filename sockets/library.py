import socket

BUFFER = 1024
SERVER_PORT = 7777


# Returns a server socket created at a given host and port.
def create_server(port):
  # Get the ipv6 information for port
  host = '::'
  info = socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE)
  # Extract socket information (addr family, socket type, protocol)
  af, socktype, protocol, canonname, socket_addr = info[0]
  # Initialize socket and bind to port
  s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
  s.bind(socket_addr)
  s.listen()
  print("Server initialized and running on port %d" % port)
  print(socket_addr)
  return s

# Returns a usable connection to a client that is accessing the server.
def connect_server(s):
  connection, addr = s.accept()
  print("Received connection from client at %s on port %s" % (addr[0], addr[1]))
  return connection, addr

# Executes command on server and/or proxy based on mode
def process_command(filepath, connection):
  # Fetch file and return
  try:
    filepath = filepath.decode().strip()
    with open(filepath, 'rb') as f:
      # Send file in buffer sized portions
      l = f.read(BUFFER)
      while (l):
        connection.send(l)
        # Output data sent on iteration
        l = f.read(BUFFER)
      # Alert client that file transfer is done
      return b'Success : File sent.'
  except FileNotFoundError:
      return b'Error : File not found.'
  except:
      return b'Error : Trouble processing request.'
