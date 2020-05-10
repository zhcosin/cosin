# 一个简易的 lisp 解释器，
# 参考文章
#     - https://zhuanlan.zhihu.com/p/28989326
#     - https://zhuanlan.zhihu.com/p/29186794

import math
import operator as op

Symbol = str          # Scheme符号由Python str表示
List = list         # Scheme列表由Python list表示
Number = (int, float)  # Scheme数字由Python的整数或浮点数表示


def tokenize(chars):
    return chars.replace('(', ' ( ').replace(')', ' ) ').split()


def atom(token):
    try:
        return int(token)
    except ValueError:
        try:
            return float(token)
        except ValueError:
            return Symbol(token)


def read_from_tokens(tokens):
    tokencount = len(tokens)

    if tokencount == 0:
        raise SyntaxError('unexpected EOF while reading')
    elif tokencount == 1:
        theonlytoken = tokens[0]
        if '(' == theonlytoken or ')' == theonlytoken:
            raise SyntaxError('single token express must be atom')
        else:
            return atom(theonlytoken)
    else:
        firsttoken = tokens[0]
        if '(' == firsttoken:
            lasttoken = tokens[-1]
            if ')' != lasttoken:
                raise SyntaxError('express must end with right parentheses')

            secondtoken = tokens[1]
            if '(' == secondtoken or ')' == secondtoken:
                raise SyntaxError('the second token of express must not be parentheses')

            L = []
            i = 1
            while i < tokencount-1:
                curtToken = tokens[i]
                if '(' == curtToken:
                    endmatchIndex = -1
                    parenthesesLevel = 1
                    for subindex in range(i+1, tokencount-1):
                        tokenAtPt = tokens[subindex]
                        if '(' == tokenAtPt:
                            parenthesesLevel += 1
                        elif ')' == tokenAtPt:
                            parenthesesLevel -= 1
                            if 0 == parenthesesLevel:
                                endmatchIndex = subindex
                                break

                    if 0 != parenthesesLevel:
                        raise SyntaxError('miss ) when determine sub express')
                    else:
                        L.append(read_from_tokens(tokens[i:(endmatchIndex+1)]))
                        i = endmatchIndex+1
                        
                else:
                    L.append(atom(curtToken))
                    i += 1
            
            return L

        elif ')' == firsttoken:
            raise SyntaxError('unexpected )')
        else:
            raise SyntaxError('express must begin with left parentheses')


def apply(f, *args, **kw):
    return f(*args, **kw)


def standard_env():
    env = {}
    env.update(vars(math))
    env.update({
        '+': op.add,
        '-': op.sub,
        '*': op.mul,
        '/': op.truediv,
        '>': op.gt,
        '<': op.lt,
        '>=': op.ge,
        '<=': op.le,
        '=': op.eq,
        'abs': abs,
        'append': op.add,
        'apply': apply,
        'begin': lambda *x: x[-1],
        'car': lambda x: x[0],
        'cdr': lambda x: x[1:], 
        'cons': lambda x,y: [x] + y,
        'eq?': op.is_, 
        'equal?': op.eq, 
        'length': len, 
        'list': lambda *x: list(x), 
        'list?': lambda x: isinstance(x,list), 
        'map': map,
        'max': max,
        'min': min,
        'not': op.not_,
        'null?': lambda x: x == [], 
        'number?': lambda x: isinstance(x, Number),   
        'procedure?': callable,
        'round':   round,
        'symbol?': lambda x: isinstance(x, Symbol),
    })

    return env


global_env = standard_env()


def eval(x, env=global_env):
    if isinstance(x, Symbol):
        return env[x]
    elif isinstance(x, Number):
        return x
    elif isinstance(x, List):
        if len(x) == 0:
            raise RuntimeError('can not eval for empty express')

        # 第一个元素必须是符号
        firstEle = x[0]
        if not isinstance(firstEle, Symbol):
            raise RuntimeError('first element is not a symbol')

        if 'define' == firstEle:
            if len(x) != 3:
                raise RuntimeError('define express must own 3 element')
            symbolvalue = eval(x[2], env)
            # print('bind symbol {} to value {}'.format(x[1], symbolvalue))
            env[x[1]] = symbolvalue
            return symbolvalue
        elif 'if' == firstEle:
            if len(x) < 4:
                raise RuntimeError('if express must own at least 3 element')
            return eval(x[2], env) if eval(x[1], env) else eval(x[-1], env)

        else:
            thecallfun = None
            try:
                thecallfun = env[firstEle]
            except KeyError:
                raise RuntimeError('symbol ' + firstEle + ' not found')
    
            # 先对参数列表逐个求值
            argumentlist = x[1:]
            argumentvaluelist = [eval(argument, env) for argument in argumentlist]

            # print('funcall: {}'.format(firstEle))
            # print('apply {} to arguments: {}'.format(firstEle, argumentvaluelist))
            return thecallfun(*argumentvaluelist)


def evalStr(express):
    tokens = tokenize(express)
    abt = read_from_tokens(tokens)
    return eval(abt)


def repl():
    print('repl for lisp:')

    while True:
        inputStr = input('>>> ')
        if 'quit' == inputStr or 'exit' == inputStr:
            return
        
        print(evalStr(inputStr))


if __name__ == '__main__':
    repl()

