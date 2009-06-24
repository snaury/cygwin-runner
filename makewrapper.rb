#!/usr/bin/ruby
require 'fileutils'

def die(msg)
  STDERR.puts msg
  exit 1
end

$EXETEMPLATE15 = "cygwin-runner-win32.exe"
$EXETEMPLATE17 = "cygwin-runner-win32-17.exe"
$EXETEMPLATE = $EXETEMPLATE15
$TARGETDIR = "/bin-public"

def makewrapper(exename = "", target = "")
  die "Usage: makewrapper.rb exename [target]" unless exename && !exename.empty?
  exename = File.join($TARGETDIR, exename.strip)
  FileUtils.mkdir_p File.dirname(exename)
  target = target.strip
  data = File.open($EXETEMPLATE, "rb") do |f|
    f.read
  end
  data << target
  data << [target.size].pack('I')
  File.open(exename, "wb") do |f|
    f.write data
    f.chmod 0755
  end
rescue
  die "Error: #{$!}"
end

while ARGV[0] =~ /\A\-\-/
  case arg = ARGV.shift
  when "--1.7"
    $EXETEMPLATE = $EXETEMPLATE17
  when "--1.5"
    $EXETEMPLATE = $EXETEMPLATE15
  else
    die "Error: unsupported argument #{arg}"
  end
end

makewrapper *ARGV
