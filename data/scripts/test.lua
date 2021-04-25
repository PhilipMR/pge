function subroutine()
	bla.print("Doddn!")
end

function onUpdate(dt)
	if testIsQButtonDown() then
		subroutine()
	end
end



