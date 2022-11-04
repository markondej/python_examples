def get_bytes(filename):
    with open(filename, "rb") as file:
        content = file.read()
    byte  = []
    for char in content:
        byte.append(char)
    return byte
    
stream = get_bytes("file.txt")
print(stream)