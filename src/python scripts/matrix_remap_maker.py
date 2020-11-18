from collections import deque
from pprint import pprint

# col2-btn = 151
# a = [
#     [1, 151, 0],
#     [1, 151, 0],
#     [0, 152, 0],
#     [1, 151, 0],
#     [1, 151, 0],
#     [1, 151, 0],
#     [1, 151, 0],
#     [1, 151, 0],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
#     [76, 51, 25],
# ]

a = [
    [68, 58 ,26],
    [65, 61 ,26],
    [62, 64 ,26],
    [58, 68 ,26],
    [54, 72 ,26],
    [51, 75 ,26],
    [48, 78 ,26],
    [45,26,4,51,26],
    [41,26,8,51,26],
    [38,26,11,51,26],
    [35,26,14,51,26],
    [32,26,18,51,26],
    [29,26,20,51,26],
    [25,25,25,51,26], #8
    [21,25,29,51,26],
    [18,25,32,51,26],
    [15,25,35,51,26],
    [12,25,38,51,26], 
    [ 9,25,41,51,26],
    [ 6,25,44,51,26],
    [ 3,25,47,51,26],
]

i = 1617
new_a = []
for r_it, row in enumerate(reversed(a)):
    temp_row = deque()
    if r_it%2==0:
        for n in range(152):
            if n < row[0]:
                temp_row.append('  -1')
            elif n < row[0] + row[1]:
                temp_row.append(str(i))
                i += 1
            elif n < row[0] + row[1] + row[2]:
                temp_row.append('  -1')
            elif n < row[0] + row[1] + row[2] + row[3]:
                temp_row.append(str(i))
                i += 1
            elif n < row[0] + row[1] + row[2] + row[3] + row[4]: 
                temp_row.append('  -1')
    else:
        row = row[::-1]
        for n in range(152):
            if n < row[0]:
                temp_row.appendleft('  -1')
            elif n < row[0] + row[1]:
                temp_row.appendleft(str(i))
                i += 1
            elif n < row[0] + row[1] + row[2]:
                temp_row.appendleft('  -1')
            elif n < row[0] + row[1] + row[2] + row[3]:
                temp_row.appendleft(str(i))
                i += 1
            elif n < row[0] + row[1] + row[2] + row[3] + row[4]: 
                temp_row.appendleft('  -1')
    new_a.append(list(temp_row))
    
with open('map.txt', 'w') as f:
    for a in new_a[::-1]:
        f.write(','.join(a))
        f.write('\n')