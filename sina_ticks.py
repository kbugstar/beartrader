#!python
# -*- coding:utf-8 -*-

import requests
import sys
import logging
import re
import json
import pandas as pd

URL_TOTAL_SYMBOLS_LITE = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getNameList?page=1&num=10000&sort=symbol&asc=1&node=hs_a'

USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_1) AppleWebKit/604.3.5 (KHTML, like Gecko) Version/11.0.1 Safari/604.3.5'
REQUESTS_HEADERS = { 'User-Agent' : USER_AGENT } 
RE_TOTAL_COUNT = re.compile(r'\(new String\("([^"]+)"\)\)')

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
    r = s.get(download_url, headers=headers)
    if r.text.startswith('<script'):
        logging.info(r.content)
        return None
    text = r.content.decode('gbk')
    return pd.read_table(pd.compat.StringIO(text))


def main():
    FORMAT = '%(asctime)-15s %(message)s'
    logging.getLogger('requests').setLevel(logging.CRITICAL)
    logging.getLogger('urllib3').setLevel(logging.CRITICAL)
    logging.basicConfig(level=logging.INFO, format=FORMAT)
    s = requests.Session()
    #total_symbols = get_active_symbols(s, 'hs_a')
    #print(total_symbols)
    total_symbols = get_all_symbols_lite(s)
    total_ticks = {}
    for x in total_symbols:
        total_ticks[x] = download_symbol_ticks(s, '2017-12-05', x)
        logging.info('download %s', x)
    sys.system("pause")

if __name__ == '__main__':
    sys.exit(main())