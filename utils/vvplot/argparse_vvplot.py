#!/usr/bin/python

import argparse
from decimal import Decimal as decimal

my_epilog="""Additional environmental variables:
VV_PREC_HI - horiz resolution of temperature and pressure fields (default 500; affects -opPt)
VV_PREC_LO - horiz resolution of streamlines (default 200; affects -s)
VV_VORT_RANGE - change contrast of vorticity field (default 50; affects -g)
VV_EPS_MULT - smooth temperature field (default 2; affects -gts)
VV_BODY_TEMP - body surface temperature (default 1; affects -t)"""

parser = argparse.ArgumentParser(description='Plot binary vvhd file',
	epilog=my_epilog, formatter_class=argparse.RawDescriptionHelpFormatter);

parser.add_argument(
	'input_file',
	type=argparse.FileType('r'),
	#dest='input',
	#action='store_true',
	help='input file'
)
parser.add_argument(
	'output_file',
	#dest='output',
	#action='store_true',
	help='output file or directory'
)

################################################################################

parser.add_argument(
	'-b',
	dest='plot_body',
	action='store_true',
	help='plot body'
)
parser.add_argument(
	'-g',
	dest='plot_vorticity',
	action='store_true',
	help='plot vorticity'
)
parser.add_argument(
	'-H',
	dest='plot_heat',
	action='store_true',
	help='plot heat particles'
)
parser.add_argument(
	'-i',
	dest='plot_ink',
	action='store_true',
	help='plot ink (streaklines)'
)
parser.add_argument(
	'-p',
	dest='plot_pressure',
	action='store_true',
	help='plot pressure field'
)
parser.add_argument(
	'-P',
	dest='plot_isopressure',
	action='store_true',
	help='plot pressure isolines'
)
parser.add_argument(
	'-s',
	dest='plot_streamlines',
	action='store_true',
	help='plot streamlines'
)
parser.add_argument(
	'-v',
	dest='plot_vortexes',
	action='store_true',
	help='plot vortex domains with dots'
)
parser.add_argument(
	'-V',
	dest='plot_vortexes_bold',
	action='store_true',
	help='plot vortex domains in bold'
)

################################################################################

parser.add_argument(
	'-w', '--grayscale',
	dest='mode_bw',
	action='store_true',
	help='plot in grayscale'
)
parser.add_argument(
	'-f', '--force',
	dest='mode_force',
	action='store_true',
	help='update fields files even if they exist'
)
parser.add_argument(
	'-n', '--dry',
	dest='mode_dry',
	action='store_true',
	help='plot fields only, dont produce output picture'
)

parser.add_argument(
	'--nooverride',
	dest='mode_nooverride',
	action='store_true',
	help='do not override output if it already exists'
)

parser.add_argument(
	'--colorbox',
	dest='mode_colorbox',
	action='store_true',
	help='draw colorbox on bottom of plot'
)

parser.add_argument(
	'--timelabel',
	dest='mode_timelabel',
	action='store_true',
	help='draw time label in top left corner'
)

parser.add_argument(
	'--holder',
	dest='plot_holder',
	action='store_true',
	help='draw body holder'
)

parser.add_argument(
	'--spring',
	dest='plot_spring',
	action='store_true',
	help='draw body spring'
)

parser.add_argument(
	'--blankbody',
	metavar="N",
	type = int,
	default = 0,
	help='do not fill body (numeration starts with 1)'
)

################################################################################
def axis_range(string):
	if len(string) == 0:
		return float('NaN')
	try:
		result = float(string)
	except ValueError:
		msg = "invalid float value: %r" % string
		raise argparse.ArgumentTypeError(msg)
	return result

parser.add_argument(
	'-x',
	dest='size_x',
	nargs=2,
	type=float,
	metavar=('XMIN', 'XMAX'),
	required=True,
	help='X axis constraints'
)
parser.add_argument(
	'-y',
	dest='size_y',
	nargs=2,
	type=axis_range,
	default=(float('NaN'),float('NaN')),
	metavar=('YMIN', 'YMAX'),
	help='Y axis constraints, either YMIN or YMAX may be an empty string'
)

################################################################################
def decimal_value(string):
	try:
		result = decimal(string)
	except Exception:
		msg = "invalid decimal value: %r" % string
		raise argparse.ArgumentTypeError(msg)
	return result

parser.add_argument(
	'--isopsi',
	# dest='isopsi',
	nargs=3,
	type=decimal_value,
	action='append',
	default=[],
	metavar=('MIN', 'MAX', 'STEP'),
	help='isolines values of streamfunction field (streamlines)'
)

parser.add_argument(
	'--psi-range',
	dest='psi_range',
	nargs=2,
	type=float,
	default=[-1.5, 1],
	metavar=('PSIMIN', 'PSIMAX'),
	help='Pressure range to be plotted in colour'
)

################################################################################
def picture_size(string):
	try:
		w_str,h_str = string.split('x')
		if (w_str == ''): w_str='0'
		elif (h_str == ''): h_str='0'
		result = (int(w_str), int(h_str))
		if result[0] < 0 or result[1] <0:
			raise ValueError;
	except ValueError:
		msg = "invalid size: %r" % string
		raise argparse.ArgumentTypeError(msg)
	return result

parser.add_argument(
	'--size',
	dest='size_pic',
	metavar='WxH',
	default='1280x720',
	type=picture_size,
	help='output figure size, either W or H can be ommited'
)

################################################################################
## REF FRAMES OPTIONS
parser.add_argument(
#	'-F',
	'--referenceframe',
#	metavar='REFFRAME',
#	dest='referenceframe'
	choices='obf',
	default='o',
	help='reference frame (original/body/fluid), default:  \'%(default)s\''
)

parser.add_argument(
	'--streamlines',
#	metavar='REFFRAME',
#	dest='streamlines'
	choices='obf',
	default='o',
	help="""choose streamlines reference frame (original/body/fluid),
	      default:  \'%(default)s\'"""
)

parser.add_argument(
	'--pressure',
#	metavar='REFFRAME',
#	dest='pressure'
	choices='sobf',
	default='s',
	help="""choose pressure mode (static pressure, original/body/fluid refframe),
	        default:  \'%(default)s\'"""
)

################################################################################
parser.add_argument(
	'--tree',
	metavar='FILE',
	dest='tree',
	help='draw tree nodes from file'
)

parser.add_argument(
	'--debug',
	dest='debug',
	action='store_true',
	help='show some debug information'
)


args = parser.parse_args()

################################################################################
def drange(start, stop, step):
	if start > stop:
		raise ValueError("MAX must be greater than MIN")
	if step <= 0:
		raise ValueError("STEP must be positive")
	result = []
	x = start
	while x<stop:
		result.append(str(x))
		x+=step
	return result

if not len(args.isopsi):
	args.isopsi += [[decimal('-10'), decimal('10'), decimal('0.1')]]

try:
	psi = []
	for i in args.isopsi:
		psi += drange(*i)
	args.isopsi = " ".join(psi)
except Exception as s:
	parser.print_usage()
	print('{}: error: argument --isopsi: {}'.format(parser.prog, str(s)))
	exit()

del drange
