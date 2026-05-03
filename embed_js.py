#!/usr/bin/env python3
import os

# Ellenőrzés: létezik-e a JS fájl
js_file = "xos-usersfs/scripts/hello.js"
if not os.path.exists(js_file):
    print(f"Warning: {js_file} not found, using default code")
    content = "let x = 42;"
else:
    with open(js_file, "r") as f:
        content = f.read()

# Escape-eljük a speciális karaktereket
content = content.replace('\\', '\\\\')
content = content.replace('"', '\\"')
content = content.replace('\n', '\\n')
content = content.replace('\r', '')

# Generáljuk a header fájlt
with open("js/embedded.h", "w") as out:
    out.write('#ifndef EMBEDDED_H\n')
    out.write('#define EMBEDDED_H\n\n')
    out.write(f'const char* EMBEDDED_JS = "{content}";\n\n')
    out.write('#endif\n')

print(f"✓ Embedded: {len(content)} characters from {js_file}")