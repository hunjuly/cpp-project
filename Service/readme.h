#pragma once
/*
https://ffmpeg.org/ffmpeg-filters.html ���� ����
  https://trac.ffmpeg.org/wiki/HowToBurnSubtitlesIntoVideo

	  Fonts �������� �־�� �Ѵ�.

		  //�ڸ� ����
		  ffmpeg - y - ss 600 - i 1.mkv - t 120 - an - vn - map 0 : 3 - c : s : 0 srt 1.srt

		  �ڸ� �����ؼ� ���ڵ� �ϸ� �� ���´�.�����ڵ�� ��ȯ�ϵ� �ƴϵ� �� �ȴ�.�Ʒ� ����� �� �Ǵ°Ŵ�.�ܺ� �ڸ�
		  ffmpeg.exe - y - ss 580 - i 1.mkv - vf subtitles = 1.srt : charenc = UTF - 8 - t 120 - map 0 : v - map 0 : a - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 640x480 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  �Ʒ�ó�� �ϸ� seek�� �ȴ�.���� �ڸ� ����
		  ffmpeg.exe - y - ss 600 - i 1.mkv - vf setpts = PTS + 600 / TB, subtitles = 1.mkv : si = 3 : charenc = UTF - 8, setpts = PTS - STARTPTS - t 120 - map 0 : v - map 0 : a - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 640x480 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  �̹��� �ڸ��ε� �κкκ� ©����.
		  ����� �����ؾ� �Ѵ�.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:v][0:s:0]overlay[v]" - t 120 - map "[v]" - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1920x1080 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  �̷��� �ϸ� �ڸ� �� ©���� �� ���´�.�ػ󵵸� �����ؾ� �ߴ�.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:s:0]scale=width=1920:height=800[sub];[0:v][sub]overlay" - t 120 - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1920x800 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4


		  �̷��� �ڸ��� �ػ󵵴� ������ �������� �ػ󵵸� ����� �Ѵ�.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:s:0]scale=width=1920:height=800[sub];[0:v][sub]overlay" - t 120 - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1280x720 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4


		  �ڸ� ũ���ϰ� ���� ũ���ϰ� �ٸ� ��찡 �ִ�.�̷��� �ڸ��� ©���ų� �߸��� ��ġ�� ����.

		  */