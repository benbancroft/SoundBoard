#!/usr/bin/env python3
 
import speech_recognition as sr
import os
import re

p = re.compile('^(.*)[\\\-]', re.IGNORECASE)

while True:

	soundboard_dir = os.environ['SOUNDBOARD']
	for file in os.listdir(soundboard_dir):

		name_match = p.match(file)
		name = ""
		if name_match:
			name = name_match.group(1)
			name = name.replace("-", "+")
			name = name.replace("_", "/")

			# Record Audio
			r = sr.Recognizer()
			try:
				with sr.AudioFile(soundboard_dir + "/" + file) as source:
					audio = audio = r.record(source)

				# Speech recognition using Google Speech Recognition
				# for testing purposes, we're just using the default API key
				# to use another API key, use `r.recognize_google(audio, key="GOOGLE_SPEECH_RECOGNITION_API_KEY")`
				# instead of `r.recognize_google(audio)`
				print(name + " said: " + r.recognize_google(audio))
			except sr.UnknownValueError:
				pass
				#print("Google Speech Recognition could not understand audio")
			except sr.RequestError as e:
				print("Could not request results from Google Speech Recognition service; {0}".format(e))
			except EOFError:
				continue	

		os.remove(soundboard_dir + "/" + file)
