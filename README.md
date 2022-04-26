Notebook binder: [![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/kaspermunch/phase-type-distributions/HEAD?labpath=Notebooks%2Fshowcase.ipynb)


To create conda environment (needs to only use conda-forge packages to make compiler work):

	conda create --override-channels -n phasetype -c conda-forge seaborn pandas r-rcpp python=3 pytables r-essentials jupyterlab rpy2 r-devtools graphviz python-graphviz rstudio

	conda install -c r rstudio