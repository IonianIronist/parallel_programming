with open('output.txt', 'r') as output:
    lines = [float(line) for line in output.readlines()]
    with open('mean.txt', 'w') as mean:
        mean.write(str(sum(lines)/len(lines)))
    print(len(lines))
