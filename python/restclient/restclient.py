# -*- coding: utf-8 -*-

"""
简易 REST 客户端，所有的 REST API 请求填写在 json 格式的配置文件中，
程序可以列出清单并执行某个请求, 配置文件中的API请求按树形结构组织.

本文件同目录下的 config.json 为配置文件，其中可以按层次结构配置 REST API 请求。
本程序的使用方式:
1. 查看API清单:
python -u restclient.py ls
2. 发起指定的API请求:
python -u restclient.py 分组1名称 分组2名称 ... API名称
这里的分组名称和API名称，是指配置文件中对应配置项的 name 属性的值.
"""

import io
import sys
import os
import traceback
import json
import requests
from urllib import request,parse

# 指定控制台输出UTF8编码
sys.stdout = io.TextIOWrapper(sys.stdout.buffer,encoding='utf-8')


def get(url, headers, params):
    "发起 GET 请求"
    print("GET {}\nheaders: {}\nparams:\n{}\n".format(url, headers, params))

    querystring = parse.urlencode(params) #查询参数
    response = requests.get(url + '?' + querystring)
    print("status_code: {}".format(response.status_code))
    print("response body: \n{}".format(response.text))
    return response.status_code, response.text

def post(url, headers, body):
    "发起 POST 请求"
    print("POST {}\nheaders: {}\nbody:\n{}\n".format(url, headers, body))

    response = requests.post(url, data=json.dumps(body), headers=headers)
    print("status_code: {}".format(response.status_code))
    print("response body: \n{}".format(response.text))
    return response.status_code, response.text

def loadRestApiFromJsonFile(configfile):
    "从指定json格式的文件加载REST接口配置形成字典"
    with open(configfile, 'r', encoding='UTF-8') as f:
        return json.load(f)

def findRestApi(data, *apipath):
    "从REST接口字典中查找指定路径的接口，路径以分组和API的名称进行标识"
    
    base_url = data["base-url"]
    restapi = data["rest-api"]

    fullurl = base_url
    found = False

    pathCount = len(apipath)
    for i in range(pathCount):
        pathitem = apipath[i]
        pathitemFound = False

        for restitem in restapi:
            apiname = restitem["name"]
            if pathitem == apiname:
                pathitemFound = True
                fullurl = fullurl + restitem["url"]
                if i == pathCount - 1:
                    restapi = restitem
                elif i == pathCount - 2:
                    restapi = restitem["api"]
                else:
                    restapi = restitem["children"]

                break

        if not pathitemFound:
            print("ERROR: path {} not found".format(pathitem))
            return (None,None)

    return restapi, fullurl

def callRestApi(restApi, fullurl):
    "调用REST API"
    headers = {"content-type": "application/json"}

    method = restApi["method"].lower()
    if method=='post':
        post(fullurl, headers, restApi.get("body", {}))
    elif method=='get':
        get(fullurl, headers, restApi.get("params", {}))
            

def listRestApi(restList, level):
    "列出全部REST接口名称及描述，并保持清晰的层次结构"
    for restItem in restList:
        message = "{}{:50}\t{}".format("    "*level, restItem["name"], restItem["description"])
        print(message)
        
        apilist = restItem.get("api", [])
        if apilist:
            listRestApi(apilist, level + 1)

        children = restItem.get("children", [])
        if children:
            listRestApi(children, level + 1)
            
        
def restClient(*argv):
    "客户端主入口，根据命令行参数决定具体的行为"
    usage = '''USAGE:
list all API from configure file with its level indent:
    python -u restclient.py config.json ls
call rest api by level path in configure file:
    python -u restclient.py ApiLevel1Name ApiChildLevel2Name ... ApiName"'''

    argc = len(argv)
    if (argc < 2):
        print("ERROR: 2 argument need be provide at least.")
        print(usage)
        return ()

    curtDir = os.getcwd()
    configfile = "{}/{}".format(curtDir, argv[0])
    print("will load api from file: {}".format(configfile))
    data = loadRestApiFromJsonFile(configfile)
    
    cmd = argv[1].lower()
    if cmd == 'ls':
        listRestApi(data["rest-api"], 0)
    elif cmd=='call':
        if argc <= 2:
            print("ERROR: No api path argument provided.")
            print(usage)
            return()

        restApi, fullurl = findRestApi(data, *argv[2:])
        if restApi:
            callRestApi(restApi, fullurl)
    elif cmd=='show':
        if argc <= 2:
            print("ERROR: No api path argument provided.")
            print(usage)
            return()

        restApi, fullurl = findRestApi(data, *argv[2:])
        if restApi:
            print("api found.")
            print("url: " + fullurl)
            print(restApi)
        else:
            print("ERROR: api NOT found, use ls show all api please.")
    else:
        print("ERROR: unknown command: {}".format(cmd))
        print(usage)
    
        
restClient(*sys.argv[1:])
