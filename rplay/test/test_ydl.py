#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import math, os
from youtube_dl import YoutubeDL

url = 'https://www.youtube.com/watch?v=2eZcYbXrI5A'
_filename = ''  # download and postprocessed media filename


def main():
	postprocessors = [{
		'key': 'FFmpegExtractAudio',
		'preferredcodec': 'best' 
	}]

	ydl_opts = {
		'format': 'bestaudio/best',
		'postprocessors': postprocessors,
		#'verbose' : True,
		'quiet' : True,
		'progress_hooks' : [progress_hooks],
		'outtmpl' : u'%(title)s-%(id)s.%(ext)s',
	}

	with YoutubeDL(ydl_opts) as ydl:
		result = ydl.download([url])
		print('result=%s' % (result, ))

	# the issue with posprocessing is that `filename` from `progress_hooks` 
	# function can no longer match downloaded file
	print('searching for resulting file ...')
	name_without_ext = os.path.splitext(_filename)[0]
	candidates = [p for p in os.listdir('.') if p.startswith(name_without_ext)]
	if len(candidates) == 1:
		print('resulting file is "%s"' % candidates[0])
	else:
		# if we have more candidates select the first one not to have `.webm` extension, 
		# however this should only happen in case of error (downloaded file not deleted by ydl)
		for f in candidates:
			if os.path.splitext(f)[1] != '.webm':
				print('resulting file is "%s' % f)


	print('done!')

def progress_hooks(args):
	"""
	progress_hooks:    A list of functions that get called on download
                       progress, with a dictionary with the entries
                       * status: One of "downloading", "error", or "finished".
                                 Check this first and ignore unknown values.

                       If status is one of "downloading", or "finished", the
                       following properties may also be present:
                       * filename: The final filename (always present)
                       * tmpfilename: The filename we're currently writing to
                       * downloaded_bytes: Bytes on disk
                       * total_bytes: Size of the whole file, None if unknown
                       * total_bytes_estimate: Guess of the eventual file size,
                                               None if unavailable.
                       * elapsed: The number of seconds since download started.
                       * eta: The estimated time in seconds, None if unknown
                       * speed: The download speed in bytes/second, None if
                                unknown
                       * fragment_index: The counter of the currently
                                         downloaded video fragment.
                       * fragment_count: The number of fragments (= individual
                                         files that will be merged)

                       Progress hooks are guaranteed to be called at least once
                       (with status "finished") if the download is successful.
	"""

	if args['status'] == 'downloading':
		progress = math.floor(float(args['downloaded_bytes']) / float(args['total_bytes']) * 100.0)
		print('downloaded %d%%' % (progress, ))
	elif args['status'] == 'finished':
		global _filename
		_filename = args['filename']
		print("downloading '%s' finished" % _filename)


if __name__ == '__main__':
	main()