#!python
import requests
import sys
import logging
import re
import json
import pandas as pd

USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_1) AppleWebKit/604.3.5 (KHTML, like Gecko) Version/11.0.1 Safari/604.3.5'
REQUESTS_HEADERS = { 'User-Agent' : USER_AGENT } 
RE_TOTAL_COUNT = re.compile(r'\(new String\("([^"]+)"\)\)')

def get_active_symbols():
    headers = REQUESTS_HEADERS
    headers['Referer'] = 'http://vip.stock.finance.sina.com.cn/mkt/'
    r = requests.get('http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeStockCount?node=sh_a', headers=headers)
    logging.info(r.text)
    found = RE_TOTAL_COUNT.match(r.text)
    if found is None:
        raise 'Bad getHQNodeStockCount response'
    symbols_count = int(found.group(1))
    if symbols_count == 0:
        raise 'Bad getHQNodeStockCount response size:%d' % symbols_count
    page_num = 1
    for i in range(0, symbols_count, 80):
        url = 'http://vip.stock.finance.sina.com.cn/quotes_service/api/json_v2.php/Market_Center.getHQNodeData?page=%d&num=80&sort=symbol&asc=1&node=sh_a&symbol=&_s_r_a=init' % page_num
        r = requests.get(url, headers=headers)
        text = r.text
        reg = re.compile(r'(\[\{|\,\{?)([^:]+)\:')
        text = reg.sub(r'\1"\2":', text)
        #text = text.replace('"{symbol', '{"symbol')
        #text = text.replace('{symbol', '{"symbol"')
        print(type(text))
        jstr = json.dumps(text)
        json_data = json.loads(jstr)
        print(json_data)
        df = pd.DataFrame(pd.read_json(json_data, dtype={'code':object}))
        print(df)
        page_num += 1



def main():
    FORMAT = '%(asctime)-15s %(message)s'
    logging.getLogger('requests').setLevel(logging.CRITICAL)
    logging.getLogger('urllib3').setLevel(logging.CRITICAL)
    logging.basicConfig(level=logging.INFO, format=FORMAT)
    get_active_symbols()

if __name__ == '__main__':
    sys.exit(main())