import setuptools

with open("README.md", "r") as readme:
    long_description = readme.read()

setuptools.setup(
     name='moosepy',  
     version='0.1',
     scripts=[],
     author="Ryan Stonebraker, Tristan Van Cise, Dain Harmon, Frank Cline",
     author_email="ryanastonebraker@gmail.com",
     description="Controller library for robotmoose robots.",
     long_description=long_description,
   long_description_content_type="text/markdown",
     url="https://github.com/robotmoose/robotmoose",
     packages=["moosepy"],
     install_requires=[
         "requests"
     ],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: Apache Software License",
         "Operating System :: OS Independent",
     ],
 )