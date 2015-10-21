# Top-level robotmoose build and packaging script

# Top-level directory name:
top=RobotMoose


all: Mac

Mac: 
	rm -fr $(top)
	cd tabula_rasa; make
	mkdir $(top)
	mkdir $(top)/tabula_rasa
	cp -r tabula_rasa/arduino $(top)/tabula_rasa/
	cp tabula_rasa/backend $(top)/tabula_rasa
	cp tabula_rasa/config.txt $(top)/tabula_rasa
	cp tabula_rasa/README.txt $(top)/tabula_rasa
	zip -r $(top)_MacOS_`date +"%Y_%m_%d"`.zip $(top)

