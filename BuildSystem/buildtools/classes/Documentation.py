import os
import os.path
import Globals

EX_OK = getattr(os, "EX_OK", 0)

###########################################################################
# Define defaults
###########################################################################
class Documentation:
    # Methods
    def __init__(self):
        print('--> Starting Documentation Builder')
        

    ###########################################################################
    # Start the documentation builder
    ###########################################################################
    def start(self):
        # Figure out if we have LaTeX in our path, if we do, then we want to
        # build the PDF documentation as well
        if Globals.latex_path_ != "":
            self.build_latex_ = True
        else:
            return Globals.PrintError("LaTeX is not installed or cannot be detected properly. Please ensure LaTeX tools are in the system path")

        return Latex().Build()

class Latex:
    def Build(self):
        print('-- Building User Manual documentation')
        cwd = os.path.normpath(os.getcwd())
        os.chdir('../Documentation/UserManual/')
        print('-- Building CASAL.syn')
        os.system('python QuickReference.py')

        for i in range(0, 3):
            if Globals.operating_system_ != "windows":
                ########################################################################
                # LINUX - Age based models
                ########################################################################
                # Create CASAL2_Age.aux
                if os.system('pdflatex --interaction=nonstopmode CASAL2_Age') != EX_OK:
                    return False
                # Create the bibliography
                if os.system('bibtex CASAL2_Age') != EX_OK:
                    return False
                # Fix the references/citations
                if os.system('pdflatex --halt-on-error --interaction=nonstopmode CASAL2_Age') != EX_OK:
                    return False
                if os.system('makeindex CASAL2_Age') != EX_OK:
                    return False
                if not os.path.exists('CASAL2_Age.pdf'):
                    return False
            else:
                ########################################################################
                # WINDOWS - Age based models
                ########################################################################
                if os.system('pdflatex.exe --enable-installer CASAL2_Age') != EX_OK:
                    return Globals.PrintError('pdflatex failed')
                if os.system('bibtex.exe CASAL2_Age') != EX_OK:
                    return Globals.PrintError('bibtex failed')
                if os.system('pdflatex.exe --halt-on-error --enable-installer CASAL2_Age') != EX_OK:
                    return Globals.PrintError('pdflatex failed')
                if os.system('makeindex.exe CASAL2_Age') != EX_OK:
                    return Globals.PrintError('makeindex failed')
            print('-- Built the Casal2_Age User Manual')
        for i in range(0, 3):
            if Globals.operating_system_ != "windows":
                ########################################################################
                # LINUX - Length based models
                ########################################################################
                # Create CASAL2_Length.aux
                if os.system('pdflatex --interaction=nonstopmode CASAL2_Length') != EX_OK:
                    return False
                # Create the bibliography
                if os.system('bibtex CASAL2_Length') != EX_OK:
                    return False
                # Fix the references/citations
                if os.system('pdflatex --halt-on-error --interaction=nonstopmode CASAL2_Length') != EX_OK:
                    return False
                if os.system('makeindex CASAL2_Length') != EX_OK:
                    return False
                if not os.path.exists('CASAL2_Length.pdf'):
                    return False
            else:
                ########################################################################
                # WINDOWS - Length based models
                ########################################################################
                if os.system('pdflatex.exe --enable-installer CASAL2_Length') != EX_OK:
                    return Globals.PrintError('pdflatex failed')
                if os.system('bibtex.exe CASAL2_Length') != EX_OK:
                    return Globals.PrintError('bibtex failed')
                if os.system('pdflatex.exe --halt-on-error --enable-installer CASAL2_Length') != EX_OK:
                    return Globals.PrintError('pdflatex failed')
                if os.system('makeindex.exe CASAL2_Length') != EX_OK:
                    return Globals.PrintError('makeindex failed')
            print('-- Built the Casal2_Length User Manual')

        print('-- Building Contributors Guide documentation')
        cwd = os.path.normpath(os.getcwd())
        os.chdir('../../Documentation/ContributorsGuide/')
        for i in range(0, 3):
            if Globals.operating_system_ != "windows":
                ########################################################################
                # LINUX
                ########################################################################
                # Create ContributorsGuide.aux
                if os.system('pdflatex --interaction=nonstopmode ContributorsGuide') != EX_OK:
                    return False
                if not os.path.exists('ContributorsGuide.pdf'):
                    return False
            else:
                ########################################################################
                # WINDOWS
                ########################################################################
                if os.system('pdflatex.exe --enable-installer ContributorsGuide') != EX_OK:
                    return Globals.PrintError('pdflatex failed')
                if not os.path.exists('ContributorsGuide.pdf'):
                    return False
                print('-- Built the Casal2 Contributors Guide')
        return True
