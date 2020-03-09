release:
	@if [ ! -d ${RELEASE} ]; then  mkdir -p ${RELEASE};  fi

cleanobj:
	@echo remove depending files
	${RM} ${OBJ}

clean:
	${RM} ${build}
	${RM} *.o

