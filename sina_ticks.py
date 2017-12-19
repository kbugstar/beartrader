#!python
# -*- coding:utf-8 -*-

import os
import sys
import requests
import logging
import re
import json
import pandas as pd
#import pymongo
import threading
import time
import random

URL_TOTAL_SYMBOLS_LITE = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getNameList?page=1&num=10000&sort=symbol&asc=1&node=hs_a'
URL_TUSHARE_CALENDAR_DATE = 'http://file.tushare.org/tsdata/calAll.csv'

# Safari
#USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_1) AppleWebKit/604.3.5 (KHTML, like Gecko) Version/11.0.1 Safari/604.3.5'
# Chrome
USER_AGENT = 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.94 Safari/537.36'
REQUESTS_HEADERS = { 'User-Agent' : USER_AGENT } 
RE_TOTAL_COUNT = re.compile(r'\(new String\("([^"]+)"\)\)')

DOWNLOAD_TICKS_COLUMNS = ['time', 'price', 'change', 'volume', 'amount', 'type']


def get_all_symbols_lite(s):
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/mkt/'
    r = s.get(URL_TOTAL_SYMBOLS_LITE, headers=headers)
    pat = re.compile(r'symbol:"([^"]+)"')
    symbols = []
    symbols.extend(pat.findall(r.text))
    return symbols

def get_active_symbols(s, market):
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/mkt/'
    total_url = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeStockCount?node=%s' % market
    r = s.get(total_url, headers=headers)
    logging.info(r.text)
    found = RE_TOTAL_COUNT.match(r.text)
    if found is None:
        raise 'Bad getHQNodeStockCount response'
    symbols_count = int(found.group(1))
    if symbols_count == 0:
        raise 'Bad getHQNodeStockCount response size:%d' % symbols_count
    page_num = 1
    num_of_page = 80
    df = None
    for i in range(0, symbols_count, num_of_page):
        url = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?page=%d&num=%d&sort=symbol&asc=1&node=%s&symbol=&_s_r_a=init' % (page_num, num_of_page, market)
        r = s.get(url, headers=headers)
        text = r.text
        reg = re.compile(r'(\[\{|\,\{?)([^:]+)\:')
        text = reg.sub(r'\1"\2":', text)
        #text = text.replace('"{symbol', '{"symbol')
        #text = text.replace('{symbol', '{"symbol"')
        jstr = json.dumps(text)
        json_data = json.loads(jstr)
        #print(json_data)
        if df is None:
            df = pd.read_json(json_data)
        else:
            df = df.append(pd.read_json(json_data))
        page_num += 1
        logging.info('pagenum:%d', page_num)
    df.to_csv('symbols.csv')
    return df

def download_symbol_ticks(s, date, symbol):
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/quotes_service/view/vMS_tradedetail.php?symbol=%s' % symbol
    download_url = 'http://market.finance.sina.com.cn/downxls.php?date=%s&symbol=%s' % (date, symbol)
    download_ok = False
    while not download_ok:
        try:
            r = s.get(download_url, headers=headers)
            r.raise_for_status()
            download_ok = True
        except requests.RequestException as e:
            logging.error(e)
            time.sleep(300.0)
    if r.text.startswith('<script'):
        time.sleep(3.0)
        logging.info(r.content)
        return None
    text = r.content.decode('gbk')
    df = pd.read_table(pd.compat.StringIO(text), names=DOWNLOAD_TICKS_COLUMNS, skiprows=1)
    df.drop(columns=['change', 'type'], inplace=True)
    #df.insert(0, 'symbol', symbol)
    #df.insert(1, 'date', date)
    df.sort_values('time', inplace=True)
    return df.reset_index(drop=True)

def downloader(symbols, data, lock, db):
    s = requests.Session()
    while True:
        symbol = None
        lock.acquire()
        symbol = symbols.pop()
        lock.release()
        if symbol is None:
            break
        df = download_symbol_ticks(s, data, symbol)
        if df is not None:
            lock.acquire()
            logging.info('download %s (remain:%d)', symbol, len(symbols))
            lock.release()
            #db.collection.insert(df.to_dict('records'))


def main():
    FORMAT = '%(asctime)-15s %(message)s'
    logging.getLogger('requests').setLevel(logging.CRITICAL)
    logging.getLogger('urllib3').setLevel(logging.CRITICAL)
    s = requests.Session()
    #total_symbols = get_active_symbols(s, 'hs_a')
    #print(total_symbols)
    #db = None
    #db_client = pymongo.MongoClient()
    #db_client.test.collection.create_index([('symbol', 1), ('date', 1)], unique=False, name='symbol_day_index')
#    for x in total_symbols:
#        total_ticks[x] = download_symbol_ticks(s, '2017-12-06', x)
#        if total_ticks[x] is not None:
#            logging.info('download %s (%d/%d)', x, len(total_ticks), len(total_symbols))
#            db_client.test.collection.insert(total_ticks[x].to_dict('records'))
#    symbols_lock = threading.RLock()
#    thread_pool = []
#    for i in range(0, 3):
#        thread_pool.append(threading.Thread(target=downloader, args=(total_symbols, '2017-12-07', symbols_lock, db)))
#    for x in thread_pool:
#        x.start()
#    for x in thread_pool:
#        x.join()
    if len(sys.argv) < 2:
        logging.error('miss store path')
        return
    dir_path = sys.argv[1]

    if len(sys.argv) >= 3:
        logging.basicConfig(filename=sys.argv[2], filemode='a', level=logging.INFO, format=FORMAT)
        logging.info('logging file:%s', sys.argv[2])
    else:
        logging.basicConfig(level=logging.INFO, format=FORMAT)

    total_symbols = get_all_symbols_lite(s)
    total_ticks = {}

    #r = requests.get(URL_TUSHARE_CALENDAR_DATE)
    trading_day = pd.read_csv(URL_TUSHARE_CALENDAR_DATE)
    trading_day.sort_index(ascending=False, inplace=True)
    trading_day_list = trading_day[(trading_day.isOpen == 1) & (trading_day.calendarDate <= '2017-12-19')]['calendarDate'].values

    for date in trading_day_list:
        logging.info('downloading date:%s', date)
        today_path = os.path.join(dir_path, date)
        if not os.path.exists(today_path):
            os.mkdir(today_path)
        random.shuffle(total_symbols)
        for symbol in total_symbols:
            if os.path.exists(os.path.join(today_path, symbol)):
                continue
            logging.info('downloading symbols:%s', symbol)
            try:
                df = download_symbol_ticks(s, date, symbol)
                if df is not None:
                    df.to_csv(os.path.join(today_path, symbol))
                #time.sleep(1.0)
            except Exception as e:
                logging.error(e)


if __name__ == '__main__':
    sys.exit(main())