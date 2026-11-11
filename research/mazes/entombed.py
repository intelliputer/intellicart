#!/usr/bin/env python3

# Python 3
# see LICENSE file for licensing information

import argparse
import random
from pathlib import Path

def getrandombit():	return random.randint(0, 1)
def leftrandombit():	return getrandombit()
def rightrandombit():	return getrandombit()
def midrandombit():	return getrandombit()
def generated(x):	pass

def prrow(seed):
	PF12 = ''
	for i in range(8):
		if seed & 1:
			PF12 = 'XX' + PF12
		else:
			PF12 = '__' + PF12
		seed >>= 1
	PF012 = 'XXXX' + PF12

	print(PF012, PF012[::-1])

# the mystery table from Entombed

MAGIC = {
	(0b00, 0b000):	1,
	(0b00, 0b001):	1,
	(0b00, 0b010):	1,
	(0b00, 0b011):	None,		# None == random bit
	(0b00, 0b100):	0,
	(0b00, 0b101):	0,
	(0b00, 0b110):	None,
	(0b00, 0b111):	None,

	(0b01, 0b000):	1,
	(0b01, 0b001):	1,
	(0b01, 0b010):	1,
	(0b01, 0b011):	1,
	(0b01, 0b100):	None,
	(0b01, 0b101):	0,
	(0b01, 0b110):	0,
	(0b01, 0b111):	0,

	(0b10, 0b000):	1,
	(0b10, 0b001):	1,
	(0b10, 0b010):	1,
	(0b10, 0b011):	None,
	(0b10, 0b100):	0,
	(0b10, 0b101):	0,
	(0b10, 0b110):	0,
	(0b10, 0b111):	0,

	(0b11, 0b000):	None,
	(0b11, 0b001):	0,
	(0b11, 0b010):	1,
	(0b11, 0b011):	None,
	(0b11, 0b100):	None,
	(0b11, 0b101):	0,
	(0b11, 0b110):	0,
	(0b11, 0b111):	0,
}

def rowgen(lastrows, render=prrow):
	# prepend and append random bits to last row
	lastrowpadded = leftrandombit()
	lastrowpadded <<= 8
	lastrowpadded |= lastrows[-1]
	lastrowpadded <<= 1
	lastrowpadded |= rightrandombit()

	# last two bits generated in current row, initial value = 10
	lasttwo = 0b10

	newrow = 0

	# iterate from 7...0, inclusive
	for i in range(7, -1, -1):
		threeabove = (lastrowpadded >> i) & 0b111

		newbit = MAGIC[lasttwo, threeabove]
		if newbit is None:
			newbit = midrandombit()
		newrow = (newrow << 1) | newbit

		lasttwo = ( (lasttwo << 1) | newbit ) & 0b11

	# hook for verification
	generated(newrow)

	# now do postprocessing
	lastrows.append(newrow)
	lastrows = lastrows[-11:]

	# condition 1
	history = [ b & 0xf0 for b in lastrows ]
	if 0 not in history:
		if sum( [ b & 0x80 for b in history ] ) == 0:
			#print 'pp 1'
			lastrows[-1] = 0

	# condition 2
	history = [ b & 0xf for b in lastrows[-7:] ]
	if 0 not in history:
		comparator = 0
		if len(lastrows) >= 9:
			comparator = lastrows[-9]
		if sum( [ b & 1 for b in history ] ) == (comparator & 1)*7:
			#print 'pp 2'
			lastrows[-1] &= 0xf0

	if render is not None:
		render(lastrows[-1])
	return lastrows

def mazegen(rows=None, render=prrow):
	lastrows = [ 0 ]
	maze_rows = []
	while rows is None or len(maze_rows) < rows:
		lastrows = rowgen(lastrows, render)
		maze_rows.append(lastrows[-1])
	return maze_rows


def row_cells(row):
	"""Return the 20 wall/open cells displayed for one maze row."""
	half = [1, 1] + [ (row >> bit) & 1 for bit in range(7, -1, -1) ]
	return half + half[::-1]


def write_svg(rows, output, cell_size=24):
	"""Write a graphical, browser-viewable rendering of the generated maze."""
	width = 20 * cell_size
	height = len(rows) * cell_size
	parts = [
		'<?xml version="1.0" encoding="UTF-8"?>',
		'<svg xmlns="http://www.w3.org/2000/svg" '
		f'viewBox="0 0 {width} {height}" width="{width}" height="{height}">',
		f'<rect width="{width}" height="{height}" fill="#101827"/>',
	]
	for y, row in enumerate(rows):
		for x, wall in enumerate(row_cells(row)):
			if wall:
				parts.append(
					f'<rect x="{x * cell_size}" y="{y * cell_size}" '
					f'width="{cell_size}" height="{cell_size}" fill="#f2c14e"/>'
				)
	parts.append('</svg>')
	Path(output).write_text('\n'.join(parts) + '\n', encoding='utf-8')


def run_gui(cell_size=24, visible_rows=30, interval_ms=100):
	"""Open a live Tkinter display of the scrolling maze."""
	try:
		import tkinter as tk
	except ImportError as error:
		raise RuntimeError(
			'Tkinter is required for --gui. Install the python3-tk package.'
		) from error

	width = 20 * cell_size
	height = visible_rows * cell_size
	try:
		root = tk.Tk()
	except tk.TclError as error:
		raise RuntimeError(
			'Unable to open a Tk window. Run --gui from a graphical desktop session.'
		) from error
	root.title('Entombed Maze')
	canvas = tk.Canvas(root, width=width, height=height,
		background='#101827', highlightthickness=0)
	canvas.pack()

	lastrows = [0]
	display_rows = []

	def render():
		canvas.delete('all')
		for y, row in enumerate(display_rows):
			for x, wall in enumerate(row_cells(row)):
				if wall:
					canvas.create_rectangle(
						x * cell_size, y * cell_size,
						(x + 1) * cell_size, (y + 1) * cell_size,
						fill='#f2c14e', outline=''
					)

	def next_row():
		nonlocal lastrows
		lastrows = rowgen(lastrows, render=None)
		display_rows.append(lastrows[-1])
		del display_rows[:-visible_rows]
		render()
		root.after(interval_ms, next_row)

	next_row()
	root.mainloop()

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Generate an Entombed maze.')
	output = parser.add_mutually_exclusive_group()
	output.add_argument('--svg', type=Path, metavar='FILE',
		help='write a graphical SVG maze to FILE')
	output.add_argument('--gui', action='store_true',
		help='open a live graphical Tkinter maze window')
	parser.add_argument('--rows', type=int, default=50,
		help='number of rows to include in SVG output (default: 50)')
	args = parser.parse_args()
	if args.rows < 1:
		parser.error('--rows must be at least 1')

	# random.seed(12345)
	if args.svg:
		write_svg(mazegen(rows=args.rows, render=None), args.svg)
		print(f'Wrote graphical maze to {args.svg}')
	elif args.gui:
		try:
			run_gui()
		except RuntimeError as error:
			parser.error(str(error))
	else:
		mazegen()
