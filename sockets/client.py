# TCP Client
import socket
import library
import sys
import time


HOSTS = {
  1: 'fdce:1d24:321:0:e5fa:2e62:9a5a:9984',
  2: 'fdce:1d24:321:0:d480:12f9:3c23:3e3f',
  3: 'fdce:1d24:321:0:d480:12f9:3c23:3e3f'
}

# For testing purposes
#HOSTS = {
#  1: '::',
#  2: '::',
#  3: '::'
#}

def usage():
  print("Usage: python client.py <device number>")
  exit(-1)

def main():
  if len(sys.argv) != 2:
    usage()
  
  key = int(sys.argv[1])

  while (True):
    # Create a client socket to interact with server
    client_socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    client_socket.connect((HOSTS[key], library.SERVER_PORT))
    filepath = input('Filepath: ')
    filename = input('Save as: ').strip()
    # Store the time when the request was sent
    send_time = time.time()
    # Send filepath to server through client socket
    client_socket.send(filepath.encode())

    # Capture data while server is still sending it
    data = client_socket.recv(library.BUFFER)
    if 'Error' in data.decode().split():
      print(data.decode())
      break

    # Write data being received to file
    with open(filename, 'wb') as f:
      while (data):
        print('Receiving data...')
        f.write(data)
        data = client_socket.recv(library.BUFFER)
        if not data:
          receive_time = time.time()
          break
    
    # Calculate RTT
    rtt = receive_time - send_time

    # Cleanup and close
    print('File successfully saved as %s' % filename)

    # Calculate RTT
    rtt = receive_time - send_time
    print('Round Trip Time (RTT):', rtt)
    client_socket.close()

    # Prompt for reentry
    while True:
      decision = input('Continue? [y/n] > ').lower()
      if decision == 'y':
        break
      if decision == 'n':
        return
      else: continue

# Run code
main()
