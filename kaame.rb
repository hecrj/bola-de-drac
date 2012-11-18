#!/usr/bin/env ruby
maps = [:demo, :maze, :zigzag]

n = ARGV.shift.to_i
period = n / 10

players = ARGV
names = players.join(' ')

counter = {}
maps.each do |map|
  counter[map] = {}
  players.each { |player| counter[map][player] = 0 }
end

n.times do |t|
  current_match = t+1
  puts "Playing match #{current_match}" if (current_match) % period == 0

  maps.each do |map|
    system("./BolaDeDrac #{names} -i #{map}.cnf -o game.bdd 2> kaame.out > /dev/null")

    winner = `tail -n 2 kaame.out`.scan(/(\w+) got top score/)[0][0]
    counter[map][winner] += 1

    sleep 1
  end
end

counter.each do |map, count|
  puts "Results in #{map}:"

  count.each do |player, victories|
    puts "    #{player} got #{victories} victories"
  end
end
