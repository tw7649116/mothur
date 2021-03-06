/*
 *  alignmentdb.cpp
 *  Mothur
 *
 *  Created by westcott on 11/4/09.
 *  Copyright 2009 Schloss Lab. All rights reserved.
 *
 */

#include "alignmentdb.h"
#include "kmerdb.hpp"
#include "suffixdb.hpp"
#include "blastdb.hpp"

/**************************************************************************************************/
AlignmentDB::AlignmentDB(string fastaFileName, string s, int kmerSize, float gapOpen, float gapExtend, float match, float misMatch, int tid, bool writeShortcut){		//	This assumes that the template database is in fasta format, may
	try {											//	need to alter this in the future?
		m = MothurOut::getInstance();
        current = CurrentFile::getInstance();
		longest = 0;
		method = s;
		bool needToGenerate = true;
		threadID = tid;
		Utils util;
        
        long start = time(NULL);
        m->mothurOutEndLine();
        m->mothurOut("Reading in the " + fastaFileName + " template sequences...\t");	cout.flush();
        //bool aligned = false;
        int tempLength = 0;
        
        ifstream fastaFile;
        util.openInputFile(fastaFileName, fastaFile);
        
        while (!fastaFile.eof()) {
            Sequence temp(fastaFile);  util.gobble(fastaFile);
            
            if (m->getControl_pressed()) {  templateSequences.clear(); break;  }
            
            if (temp.getName() != "") {
                templateSequences.push_back(temp);
                
                //save longest base
                if (temp.getUnaligned().length() >= longest)  { longest = (temp.getUnaligned().length()+1); }
                
                if (tempLength != 0) {
                    if (tempLength != temp.getAligned().length()) { m->mothurOut("[ERROR]: template is not aligned, aborting.\n"); m->setControl_pressed(true); }
                }else { tempLength = temp.getAligned().length(); }
            }
        }
        fastaFile.close();
        
        numSeqs = templateSequences.size();
        //all of this is elsewhere already!
        
        m->mothurOut("DONE.");
        m->mothurOutEndLine();	cout.flush();
        m->mothurOut("It took " + toString(time(NULL) - start) + " to read  " + toString(templateSequences.size()) + " sequences."); m->mothurOutEndLine();  

		
		//in case you delete the seqs and then ask for them
		emptySequence = Sequence();
		emptySequence.setName("no_match");
		emptySequence.setUnaligned("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		emptySequence.setAligned("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		
		
		string kmerDBName;
		if(method == "kmer")			{	
			search = new KmerDB(fastaFileName, kmerSize);			

            kmerDBName = fastaFileName.substr(0,fastaFileName.find_last_of(".")+1) + char('0'+ kmerSize) + "mer";
            
            ifstream kmerFileTest(kmerDBName.c_str());
				
            if(kmerFileTest){
                bool GoodFile = util.checkReleaseVersion(kmerFileTest, current->getVersion());
                int shortcutTimeStamp = util.getTimeStamp(kmerDBName);
                int referenceTimeStamp = util.getTimeStamp(fastaFileName);
                
                //if the shortcut file is older then the reference file, remake shortcut file
                if (shortcutTimeStamp < referenceTimeStamp) {  GoodFile = false;  }
                
                if (GoodFile) {  needToGenerate = false;	}
            }
			
		}
		else if(method == "suffix")		{	search = new SuffixDB(numSeqs);								}
		else if(method == "blast")		{	search = new BlastDB(fastaFileName.substr(0,fastaFileName.find_last_of(".")+1), gapOpen, gapExtend, match, misMatch, "", threadID);	}
		else {
			method = "kmer";
			m->mothurOut(method + " is not a valid search option. I will run the command using kmer, ksize=8.");
			m->mothurOutEndLine();
			search = new KmerDB(fastaFileName, 8);
		}
		
		if (!m->getControl_pressed()) {
			if (needToGenerate) {
				//add sequences to search 
				for (int i = 0; i < templateSequences.size(); i++) {
					search->addSequence(templateSequences[i]);
					
					if (m->getControl_pressed()) {  templateSequences.clear(); break;  }
				}
				
				if (m->getControl_pressed()) {  templateSequences.clear();  }
				
                if ((method != "kmer") || ((method == "kmer") && (writeShortcut))) { search->generateDB(); }
                
			}else if ((method == "kmer") && (!needToGenerate)) {
				ifstream kmerFileTest(kmerDBName.c_str());
				search->readKmerDB(kmerFileTest);	
			}
		
			search->setNumSeqs(numSeqs);
		}
		
	}
	catch(exception& e) {
		m->errorOut(e, "AlignmentDB", "AlignmentDB");
		exit(1);
	}
}
/**************************************************************************************************/
AlignmentDB::AlignmentDB(string s){		 
	try {											
		m = MothurOut::getInstance();
		method = s;
		
		if(method == "suffix")		{	search = new SuffixDB();	}
		else if(method == "blast")	{	search = new BlastDB("", 0);		}
		else						{	search = new KmerDB();		}

				
		//in case you delete the seqs and then ask for them
		emptySequence = Sequence();
		emptySequence.setName("no_match");
		emptySequence.setUnaligned("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		emptySequence.setAligned("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		
	}
	catch(exception& e) {
		m->errorOut(e, "AlignmentDB", "AlignmentDB");
		exit(1);
	}
}
/**************************************************************************************************/
AlignmentDB::~AlignmentDB() {  delete search;	}
/**************************************************************************************************/
Sequence AlignmentDB::findClosestSequence(Sequence* seq, float& searchScore) {
	try{
        lock_guard<std::mutex> guard(mutex);
        vector<float> scores;
		vector<int> spot = search->findClosestSequences(seq, 1, scores);
	
        if (spot.size() != 0)	{	searchScore = scores[0]; return templateSequences[spot[0]];	}
        else					{ 	searchScore = 0; return emptySequence;                      }
		
	}
	catch(exception& e) {
		m->errorOut(e, "AlignmentDB", "findClosestSequence");
		exit(1);
	}
}
/**************************************************************************************************/






