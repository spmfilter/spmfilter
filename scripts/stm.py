#!/usr/bin/env python

import optparse
import smtplib
import email

class Option (optparse.Option):
	ATTRS = optparse.Option.ATTRS + ['required']

	def _check_required (self):
		if self.required and not self.takes_value():
			raise OptionError(
				"required flag set for option that doesn't take a value",
				self)

	CHECK_METHODS = optparse.Option.CHECK_METHODS + [_check_required]

	def process (self, opt, value, values, parser):
		optparse.Option.process(self, opt, value, values, parser)
		parser.option_seen[self] = 1

class OptionParser (optparse.OptionParser):
	def _init_parsing_state (self):
		optparse.OptionParser._init_parsing_state(self)
		self.option_seen = {}

	def check_values (self, values, args):
		for option in self.option_list:
			if (isinstance(option, Option) and
				option.required and
				not self.option_seen.has_key(option)):
				self.error("%s not supplied" % option)
		return (values, args)


if __name__ == "__main__":
	usage = "usage: %prog [options]"
	parser = OptionParser(usage=usage,option_list=[
		Option("-d", "--destination",
			action="store", type="string", dest="server",
			help="destination smtp server", required=1),
		Option("-f", "--file",
			action="store", type="string", dest="file",
			help="message FILE", required=1),
		Option("-s", "--sender",
			action="store", type="string", dest="sender",
			help="message sender", required=1),
		Option("-r", "--recipient",
			action="store", type="string", dest="recipient",
			help="message recipient", required=1),
		Option("-v", "--verbose",
			action="store_true", dest="verbose")
	])

	(options, args) = parser.parse_args()

	fp = open(options.file)
	msg = email.message_from_file(fp)
	fp.close()

	server = smtplib.SMTP(options.server)
	if options.verbose: server.set_debuglevel(1)
	server.sendmail(options.sender, [options.recipient], msg.as_string())
	server.quit()
