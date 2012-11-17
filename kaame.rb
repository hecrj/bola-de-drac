#!/usr/bin/env ruby
maps = [:demo, :maze, :zigzag]

n = ARGV.shift.to_i
cores = `grep -c processor /proc/cpuinfo`.to_i
n_core = n / cores

players = ARGV
names = players.join(' ')

counter = {}
maps.each do |map|
  counter[map] = {}
  players.each { |player| counter[map][player] = 0 }
end

pipes = {:read => [], :write => []}

cores.times do |c|
  pipes[:read][c], pipes[:write][c] = IO.pipe
  
  fork do
    pipes[:read][c].close

    n_core.times do |t|
      maps.each do |map|
        puts "Playing match #{n_core*c + t + 1} in #{map}"

        system("./BolaDeDrac #{names} -i #{map}.cnf -o game.bdd 2> kaame#{c}.out > /dev/null")

        winner = `tail -n 2 kaame#{c}.out`.scan(/(\w+) got top score/)[0][0]
        counter[map][winner] += 1
      end
    end

    Marshal.dump(counter, pipes[:write][c])
  end
end

pipes[:write].each { |pipe| pipe.close }

results = []
pipes[:read].each { |pipe| results << pipe.read }

Process.waitpid

results.each do |result|
  raise "child failed" if result.empty?

  subcounter = Marshal.load(result)

  subcounter.each do |map, subcount|
    subcount.each { |player, victories| counter[map][player] += victories }
  end
end

counter.each do |map, count|
  puts "Results in #{map}:"

  count.each do |player, victories|
    puts "    #{player} got #{victories} victories"
  end
end
