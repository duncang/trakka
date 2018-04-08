#!/bin/python

import csv
import MySQLdb

mydb = MySQLdb.connect(host='localhost',
    user='root',
    passwd='tiger228',
    db='trakka')
cursor = mydb.cursor()

csv_data = csv.reader(file('mode-s.csv'))
for row in csv_data:

#    cursor.execute('INSERT INTO icao_codes(icao_id, tail_num, index)' \
#          'VALUES("0x%s", "%s", "%s") ON DUPLICATE KEY UPDATE ', 
#           row)

	icao_int = int(row[0],16)
	print icao_int
	print hex(icao_int)

#	cursor.execute('UPDATE icao_codes SET icao_id=%s WHERE tail_num="%s"', (hex(icao_int), row[1]))
	cursor.execute('INSERT INTO icao_codes (icao_id, tail_num) values (%s, %s)', (icao_int, row[1]))
	print row
	mydb.commit()

#cursor.execute('UPDATE icao_codes SET icao_id=0x7c0001 WHERE tail_num="VH-AAB"')

#close the connection to the database.

cursor.close()
print "Done"

