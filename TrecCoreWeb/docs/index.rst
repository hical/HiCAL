.. trecCoreWeb documentation master file, created by
   sphinx-quickstart.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

<ProjectName>'s documentation!
====================================================================

<projectname> is a high-recall retrieval platform the supports different retrieval methods.
This repository contains the platform server which is responsible for displaying the document assessment interfaces to the user,
configuring the system, and communicating requests and responses to and from each retrieval component.

Architecture
^^^^^^^^^^^^
The figure below shows the high-level architecture of our system.

.. figure:: architecure.png
   :scale: 35 %
   :alt: System Architecture

   A high-level view of our system architecture.

Our platform supports different retrieval methods.

Contents:

.. toctree::
   :maxdepth: 2

   setup
   users
   deploy
   docker_ec2
   tests



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
