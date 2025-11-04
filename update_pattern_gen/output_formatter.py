input_file = open("fanda_1200.txt", "r")

res = []
for line in input_file:
    frame, index, a, b, c, d = map(int, line.strip().split())
    res.append((frame, index, (a, b, c, d)))

out = sorted(res, key=lambda x: x[0])
output_file = open(f"update_pattern_{len(res)}_sorted.txt", "w")
for frame, index, (a, b, c, d) in out:
    output_file.write(f"{frame} {index} {a} {b} {c} {d}\n")