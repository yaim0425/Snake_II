import io

path = r"D:\Documents\ESP32S3\Snake_II\PROYECTO.md"
with io.open(path, "r", encoding="utf-8") as f:
    lines = f.read().splitlines()

# Localizar la seccion "## 7. CHANGELOG" y la siguiente seccion de primer nivel
start = None
end = None
for i, l in enumerate(lines):
    if l.startswith("## 7. CHANGELOG"):
        start = i
    elif start is not None and l.startswith("## "):
        end = i
        break

assert start is not None, "no se encontro ## 7. CHANGELOG"
assert end is not None, "no se encontro la seccion siguiente"
print("CHANGELOG:", start + 1, "-", end)  # 1-indexed inclusive range to delete

# Reconstruir: descartar [start, end)
new = lines[:start] + lines[end:]

# Renumerar la seccion siguiente (era ## 8 -> ## 7)
for i in range(start, len(new)):
    if new[i].startswith("## 8. "):
        new[i] = new[i].replace("## 8. ", "## 7. ", 1)
        print("renumerada:", new[i])
        break

with io.open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(new) + "\n")
print("OK, lineas nuevas:", len(new))
