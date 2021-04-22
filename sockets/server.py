# TCP Server
import socket
import library
import threading
from _thread import *

server_lock = threading.Lock()

# Process command in a separate spawned thread
def server_thread(connection):
  # Get the path of the file in the client request
  data = connection.recv(library.BUFFER)
  # Close the thread
  if not data:
    print('Error processing command')
    connection.sendall(b'Error processing command')
    server_lock.release()
    return
  # Return the file given the filepath
  response = library.process_command(data, connection)

  # Send it back to the client through the connection
  connection.sendall(response)
  # Clean up the connection
  connection.close()
  server_lock.release()


def main():
    # Spin up the server
  s = library.create_server(library.SERVER_PORT)
  
  # Indefinitely process client requests
  while True:
    # Connect the client to the server socket
    connection, addr = library.connect_server(s)
    # Allow client to acquire the lock
    server_lock.acquire()
    print('Connected to', addr[0], ':', addr[1])
    # Fulfil request in a new thread
    start_new_thread(server_thread, (connection,))

  # Clean up the connection
  s.close()

# Run code
main()
