## quat multiplication simplifier
## https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html

## quaternions in the form of Q = (w, x, y, z) or Q = (w, i, j, k)

class math_expr:
    def __init__(self, expr = '0'):
        if type(expr) == math_expr:
            self.s = expr.s
            self.isZero = expr.isZero
            self.isOne = expr.isOne
        self.s = str(expr)
        self.isZero = self.s == '0'
        self.isOne = self.s == '1'
    
    def __mul__(self, other):
        if self.isZero or other.isZero:
            return math_expr()
        if self.isOne:
            return other
        if other.isOne:
            return self
        return math_expr(self.s + ' * ' + other.s)
    
    def __add__(self, other):
        if self.isZero and other.isZero:
            return math_expr()
        if self.isZero:
            return other
        if other.isZero:
            return self
        return math_expr(self.s + ' + ' + other.s)

    def __sub__(self, other):
        if self.isZero and other.isZero:
            return math_expr()
        if self.isZero:
            return math_expr(' - ' + other.s)
        if other.isZero:
            return self
        return math_expr(self.s + ' - ' + other.s)
    
    def __repr__(self):
        return self.s

def multiply(r: tuple, s: tuple):
    r = tuple(math_expr(x) for x in r)
    s = tuple(math_expr(x) for x in s)
    return (
        r[0] * s[0] - r[1] * s[1] - r[2] * s[2] - r[3] * s[3],
        r[0] * s[1] + r[1] * s[0] - r[2] * s[3] + r[3] * s[2],
        r[0] * s[2] + r[1] * s[3] + r[2] * s[0] - r[3] * s[1],
        r[0] * s[3] - r[1] * s[2] + r[2] * s[1] + r[3] * s[0]
    )

def printquat(q: tuple):
    print('(')
    for i, v in enumerate(q):
        print(f' {"wxyz"[i]} = {v.s.strip()}')
    print(')')

names = ["Yaw", "Pitch", "Roll"]
qidentity = (1, 0, 0, 0)
qyaw = ('cosYaw', 0, 'sinYaw', 0)
qpitch = ('cosPitch', 'sinPitch', 0, 0)
qroll = ('cosRoll', 0, 0, 'sinRoll')
quats = [qyaw, qpitch, qroll]

import itertools

header = open('euler_quats.h', 'w')
source = open('euler_quats.cpp', 'w')

header.write('namespace glm\n{\n')

for mask in range(1, 8):
    bits = [i for i in range(3) if ((mask >> i) & 1) ]
    for indices in itertools.permutations(bits):
        curNames = [names[x] for x in indices]
        lcNames = [name.lower() for name in curNames]
        print('->'.join(curNames), ':')

        funcName = f"from{''.join(curNames)}"
        funcArgs = ', '.join([f'float {x}' for x in curNames])
        funcSignature = f'{funcName}({funcArgs})'

        header.write(f"\t quat {funcSignature};\n")
        if len(indices) == 3:
            vecName = lcNames[0] + curNames[1] + curNames[2]
            funcSignatureVec = f'{funcName}(const vec3& {vecName})'
            funcSource = '{' + f' return {funcName}({vecName}.x, {vecName}.y, {vecName}.z); ' + '}'
            header.write(f"\t inline quat {funcSignatureVec} {funcSource}\n")

        source.write(f"quat glm::{funcSignature}\n")
        source.write('{\n')

        variables = [
            f'const float {trigFunc}{angle} = glm::{trigFunc}({angle} * 0.5f)'
            for angle, argName in zip(curNames, lcNames)
            for trigFunc in ('cos', 'sin')
        ]
        for var in variables:
            source.write(f'\t{var};\n')

        result = qidentity
        for idx in reversed(indices):
            result = multiply(quats[idx], result)
        source.write(f'\treturn quat(\n')
        quatBody = ',\n'.join([f'\t\t{comp.s if comp.s != "0" else "0.0f"}' for comp in result])
        source.write(f'{quatBody}\n\t);\n')
        source.write('}\n\n')

        printquat(result)
        print('---')


header.write('}\n')
header.close()
source.close()

## qyp = multiply(qpitch, qyaw)
## qypr = multiply(qroll, qyp)
## printquat(qyp)
## printquat(qypr)