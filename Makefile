all:
	mkdir -p vme_drs_v5.1_tdc/common/lib vme_drs_v5.1_tdc/daq/bin vme_drs_v5.1_tdc/analyzer/bin Analyzer/bin component/MznHRTdcReader1/standalone/lib
	$(MAKE) -C vme_drs_v5.1_tdc all;
	$(MAKE) -C daq-mw-app/util all;
	$(MAKE) -C component/MznHRTdcReader1/standalone/src all;
	$(MAKE) -C component all;
	$(MAKE) -C Analyzer/src all;

clean:
	$(MAKE) -C vme_drs_v5.1_tdc clean;
	$(MAKE) -C daq-mw-app/util clean;
	$(MAKE) -C component/MznHRTdcReader1/standalone/src clean;
	$(MAKE) -C component clean;
	$(MAKE) -C Analyzer/src clean;
