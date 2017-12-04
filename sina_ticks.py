#!python
# -*- coding:utf-8 -*-

import requests
import sys
import logging
import re
import json
import pandas as pd

USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_1) AppleWebKit/604.3.5 (KHTML, like Gecko) Version/11.0.1 Safari/604.3.5'
REQUESTS_HEADERS = { 'User-Agent' : USER_AGENT } 
RE_TOTAL_COUNT = re.compile(r'\(new String\("([^"]+)"\)\)')

def get_active_symbols(market):
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/mkt/'
    total_url = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeStockCount?node=%s' % market
    r = requests.get(total_url, headers=headers)
    logging.info(r.text)
    found = RE_TOTAL_COUNT.match(r.text)
    if found is None:
        raise 'Bad getHQNodeStockCount response'
    symbols_count = int(found.group(1))
    if symbols_count == 0:
        raise 'Bad getHQNodeStockCount response size:%d' % symbols_count
    page_num = 1
    df = None
    for i in range(0, symbols_count, 80):
        url = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?page=%d&num=80&sort=symbol&asc=1&node=%s&symbol=&_s_r_a=init' % (page_num, market)
        r = requests.get(url, headers=headers)
        text = r.text
        reg = re.compile(r'(\[\{|\,\{?)([^:]+)\:')
        text = reg.sub(r'\1"\2":', text)
        #text = text.replace('"{symbol', '{"symbol')
        #text = text.replace('{symbol', '{"symbol"')
        jstr = json.dumps(text)
        json_data = json.loads(jstr)
        #print(json_data)
        if df is None:
            df = pd.DataFrame(pd.read_json(json_data))
        else:
            df = df.append(pd.read_json(json_data))
        page_num += 1
        print('pagenum:', page_num)
    print(df)
    df.to_csv('symbols.csv')

def get_symbol_ticks(symbol):
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/quotes_service/view/vMS_tradedetail.php?symbol=%s' % symbol
    download_url = 'http://market.finance.sina.com.cn/downxls.php?date=%s&symbol=%s' % (date, symbol)




def main():
    FORMAT = '%(asctime)-15s %(message)s'
    logging.getLogger('requests').setLevel(logging.CRITICAL)
    logging.getLogger('urllib3').setLevel(logging.CRITICAL)
    logging.basicConfig(level=logging.INFO, format=FORMAT)
    get_active_symbols('hs_a')

if __name__ == '__main__':
    sys.exit(main())