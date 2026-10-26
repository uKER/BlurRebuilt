namespace Bizarre.Misc
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Net.Mail;
    using System.Text;

    /*
     * Usage example:
     * 
        Mail mail = new Mail();

        mail.Sender         = "sender@example.com";
        mail.AddRecipient("recipient@example.com");

        mail.Subject        = "Test Subject";
        mail.Body           = "Test Body of email";
        mail.IsBodyHtml     = true;

        mail.AddAttachment("c:\somefile.txt");

        mail.Send();
     */

    public class Mail
    {
        private string smtpServer = "localhost";

        private string smtpServerUsername;

        private string smtpServerPassword;

        private string subject = string.Empty;

        private string sender;
        
        private string senderDisplayName;

        private string body = string.Empty;

        private bool isBodyHtml = false;

        private bool enableSsl = false;

        private List<String> recipients = new List<string>();

        private List<String> attachments = new List<string>();

        public Mail()
        {
        }

        public string SmtpServer
        {
            get
            {
                return this.smtpServer;
            }

            set
            {
                this.smtpServer = value;
            }
        }

        public string SmtpServerUsername
        {
            get
            {
                return this.smtpServerUsername;
            }

            set
            {
                this.smtpServerUsername = value;
            }
        }

        public string SmtpServerPassword
        {
            get
            {
                return this.smtpServerPassword;
            }

            set
            {
                this.smtpServerPassword = value;
            }
        }

        public string Sender
        {
            get
            {
                return this.sender;
            }

            set
            {
                this.sender = value;
            }
        }

        public string SenderDisplayName
        {
            get
            {
                return this.senderDisplayName;
            }

            set
            {
                this.senderDisplayName = value;
            }
        }

        public string Body
        {
            get
            {
                return this.body;
            }

            set
            {
                this.body = value;
            }
        }

        public bool IsBodyHtml
        {
            get
            {
                return this.isBodyHtml;
            }

            set
            {
                this.isBodyHtml = value;
            }
        }

        public bool EnableSsl
        {
            get
            {
                return this.enableSsl;
            }

            set
            {
                this.enableSsl = value;
            }
        }

        public string Subject
        {
            get
            {
                return this.subject;
            }

            set
            {
                this.subject = value;
            }
        }

        public void AddRecipient(string to)
        {
            recipients.Add(to);
        }

        public void AddAttachment(string filename)
        {
            attachments.Add(filename);
        }

        public void Send()
        {
            System.Diagnostics.Debug.Assert(this.sender != null);

            string to = string.Empty;

            foreach (string r in recipients)
            {
                if (to.Length > 0)
                {
                    to += ",";
                }

                to += r;
            }

            MailMessage message = new MailMessage(this.sender, to);
            
            message.Subject = this.subject;

            if (this.senderDisplayName == null)
            {
                message.From = new MailAddress(this.sender);
            }
            else
            {
                message.From = new MailAddress(this.sender, this.senderDisplayName);
            }

            message.Body = this.body;
            message.IsBodyHtml = this.isBodyHtml;

            foreach (string filename in attachments)
            {
                message.Attachments.Add(new Attachment(filename));
            }

            SmtpClient smtp = new SmtpClient(this.smtpServer);
            
            smtp.EnableSsl = this.enableSsl;

            if (this.smtpServerUsername != null && this.smtpServerPassword != null)
            {
                smtp.Credentials = new System.Net.NetworkCredential(this.smtpServerUsername, this.smtpServerPassword);
            }
            
            smtp.Send(message);
        }
    }
}
