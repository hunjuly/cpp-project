#pragma once
/*
https://ffmpeg.org/ffmpeg-filters.html 여기 참고
  https://trac.ffmpeg.org/wiki/HowToBurnSubtitlesIntoVideo

	  Fonts 서브폴더 있어야 한다.

		  //자막 추출
		  ffmpeg - y - ss 600 - i 1.mkv - t 120 - an - vn - map 0 : 3 - c : s : 0 srt 1.srt

		  자막 추출해서 인코딩 하면 잘 나온다.유니코드로 변환하든 아니든 잘 된다.아래 명령은 잘 되는거다.외부 자막
		  ffmpeg.exe - y - ss 580 - i 1.mkv - vf subtitles = 1.srt : charenc = UTF - 8 - t 120 - map 0 : v - map 0 : a - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 640x480 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  아래처럼 하면 seek가 된다.내부 자막 지정
		  ffmpeg.exe - y - ss 600 - i 1.mkv - vf setpts = PTS + 600 / TB, subtitles = 1.mkv : si = 3 : charenc = UTF - 8, setpts = PTS - STARTPTS - t 120 - map 0 : v - map 0 : a - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 640x480 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  이미지 자막인데 부분부분 짤린다.
		  오디오 선택해야 한다.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:v][0:s:0]overlay[v]" - t 120 - map "[v]" - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1920x1080 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4

		  이렇게 하면 자막 안 짤리고 잘 나온다.해상도를 지정해야 했다.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:s:0]scale=width=1920:height=800[sub];[0:v][sub]overlay" - t 120 - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1920x800 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4


		  이렇게 자막쪽 해상도는 비디오의 오리지널 해상도를 적어야 한다.
		  ffmpeg - y - ss 600 - i 1.mkv - filter_complex "[0:s:0]scale=width=1920:height=800[sub];[0:v][sub]overlay" - t 120 - map 0 : a : 0 - f mov - c : a aac - strict experimental - ar 44100 - b : a 128k - ac 2 - s 1280x720 - c : v libx264 - pix_fmt yuv420p - preset ultrafast - qp 0 - r 15 - x264opts "crf=20:vbv-bufsize=1024:vbv-maxrate=1024:keyint=90:bframes=0" out.mp4


		  자막 크기하고 영상 크기하고 다른 경우가 있다.이러면 자막이 짤리거나 잘못된 위치로 간다.

		  */