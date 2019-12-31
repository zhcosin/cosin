# -*- coding: utf-8 -*-
import cx_Oracle
import pymysql
import sys
import os
import traceback
import json
import datetime
import time
from apscheduler.schedulers.background import BackgroundScheduler

def updateTimeToToday(dbProduct, datasource, tablename, baseField, updateFields):
    print("======== {} ========<".format(tablename))
    print("process table {} by field {}, updated fields: {}".format(tablename, baseField, updateFields))

    daysBeforeToday = -1

    con = None #数据库连接
    if dbProduct == 'oracle':
        con = cx_Oracle.connect(datasource) 
    elif dbProduct == 'mysql':
        con = pymysql.connect(**datasource) #mysql
    else:
        print("unknown database product: {}".format(dbProduct))
        return ()
    
    cur = con.cursor()
    sqlForDetermiterIncrement = '' # 查询时间字段最大值与当前时间的天数差的 sql 语句
    if dbProduct == 'oracle':
        sqlForDetermiterIncrement = 'select trunc(sysdate)-trunc(max({baseField})) mindaydiff from {tablename}'.format(baseField=baseField, tablename=tablename)
    elif dbProduct == 'mysql':
        sqlForDetermiterIncrement = "select timestampdiff(day, date_format(max({baseField}), '%Y-%m-%d'), date_format(now(), '%Y-%m-%d')) mindaydiff from {tablename}".format(baseField=baseField, tablename=tablename)
    print(sqlForDetermiterIncrement)
    cur.execute(sqlForDetermiterIncrement)
    res = cur.fetchall()

    recordCount = len(res)
    if recordCount > 0:
        for r in res:
            daysBeforeToday = r[0]
    else:
        print('query result is empty when determine the increment.')

    print("the diff days between max(baseField) and today is {}".format(daysBeforeToday))
        
    if daysBeforeToday > 0:
        fieldsSetSqlList = [];
        for field in updateFields:
            if dbProduct == 'oracle':
                fieldsSetSqlList.append('{field}={field}+{increment}'.format(field=field, increment=daysBeforeToday)) #oracle
            elif dbProduct == 'mysql':
                fieldsSetSqlList.append('{field}=date_add({field}, interval {increment} day)'.format(field=field, increment=daysBeforeToday)) #mysql

        fieldsSetSql = ','.join(fieldsSetSqlList)
        
        updateAppInfoSql = 'update {tablename} set {fieldsSetSql}'.format(tablename=tablename, fieldsSetSql=fieldsSetSql)
        print(updateAppInfoSql)

        cur.execute(updateAppInfoSql)
        con.commit()
    else:
        print('increment days {increment} ignored.'.format(increment=daysBeforeToday))
        
    con.close()

    print(">======== {} ========".format(tablename))

def update_date_work(data):
    print("task execute.")
    dbProduct = data['db-product']
    datasource = data['datasource']
    infolist = data['infos']
    try:
        for info in infolist:
            updateTimeToToday(dbProduct, datasource, info["tableName"], info["baseField"], info["updatedFields"])
    except Exception as e:
        print(e)
        traceback.print_exc()

def update_date(*argv):
    argc = len(argv)
    if (argc < 1):
        print("ERROR: 2 argument need be provide at least.")
        print(usage)
        return ()

    curtDir = os.getcwd()
    configfile = "{}/{}".format(curtDir, argv[0])
    print("will load api from file: {}".format(configfile))

    with open(configfile, 'r') as f:
        data = json.load(f)
        # 启动时执行一次，以后按定时器执行
        update_date_work(data)
        # 定时任务
        work = lambda : update_date_work(data)
        scheduler = BackgroundScheduler()
        scheduler.add_job(work, 'cron', hour = '0', minute = '5', second = '0') # 每天 00:05:00 定时执行
        scheduler.start()

        while True:
            print("CurrentTime: ", datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3])
            time.sleep(3600)

update_date(*sys.argv[1:])
