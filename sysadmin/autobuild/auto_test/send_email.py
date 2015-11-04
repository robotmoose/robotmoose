#! /usr/bin/python

# Python script for emailing git pull and compile issues when testing robotmoose
# The proper use of this script is:
# send_email.py <recipient_email_address> <filename_of_email_text>
#
# The localhost must be running the Postfix SMTP server in order for this script to work properly.


import sys

def send_email( recipient, textfile ): # recipient is the recipient's email address

    # Import smtplib for the actual sending function
    import smtplib

    # Import the email modules we'll need
    from email.mime.text import MIMEText

    # Open a plain text file for reading.  For this example, assume that
    # the text file contains only ASCII characters.
    fp = open(textfile, 'rb')
    # Create a text/plain message
    msg = MIMEText(fp.read())
    fp.close()

    sender = 'moosymoose@robotmoose.com'

    msg['Subject'] = 'Error messages from robotmoose tests'
    msg['From'] = sender
    msg['To'] = recipient

    # Send the message via our own SMTP server, but don't include the envelope header.
    s = smtplib.SMTP('localhost')
    s.sendmail(sender, [recipient], msg.as_string())
    s.quit()
    return

send_email(str(sys.argv[1]), str(sys.argv[2])) # Read imput arguments and feed them into send_email function
