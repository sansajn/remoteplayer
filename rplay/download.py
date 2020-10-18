#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Youtube-dl download helper script for Python 2.

import native  # rename to rplay and use as `import rplay as config`
from youtube_dl import YoutubeDL

def download_audio(url):
	"""download specified media audio using youtube-dl (for python2)
	see `/usr/local/lib/python2.7/dist-packages/youtube_dl/YoutubeDL.py` for 
	usage details and available ydl options."""

	postprocessors = []
	postprocessors.append({
		'key': 'FFmpegExtractAudio',
		'preferredcodec': 'best' 
	})

	ydl_opts = {
		'format': 'bestaudio/best',
		'postprocessors': postprocessors,
		#'verbose' : True,
		'quiet' : True,
		'progress_hooks' : [native.progress_hooks],
		'ffmpeg_location' : native.ffmpeg_location(),
		'outtmpl' : native.filesystem_output(),  # '%(title)s-%(id)s.%(ext)s'
		# logtostderr
		'cachedir' : '/tmp'
		#'download_archive' : 'history.txt'
	}

	with YoutubeDL(ydl_opts) as ydl:
		result = ydl.download([url])
